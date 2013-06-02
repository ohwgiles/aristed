#include "project.hpp"

#include <QFileDialog>
#include <QSettings>
#include <QTextBrowser>
#include <QProcess>
#include <QMessageBox>

#include "log.hpp"
AeProject::AeProject() :
	QObject(),
process_(0)
{
}

QString AeProject::displayName() const {
	if(!projectName_.isEmpty())
		return projectName_;
	else
		return sourceDir_.dirName();
}
void AeProject::setSourceDir(QDir sourceDir) {
	sourceDir_ = sourceDir;
}

void AeProject::parseAeproj(const QString projFile) {
	QSettings proj(projFile, QSettings::IniFormat);

	projectName_ = proj.value("project").toString();
	ae_info("Found project name: " << projectName_);

	QFileInfo fi(projFile);
	sourceDir_ = fi.absoluteDir();
	ae_debug("Found source dir: " << sourceDir_.absolutePath());

	QDir builddir(sourceDir_);
	builddir.cd(proj.value("builddir").toString());
	buildDir_ = builddir.absolutePath();
	ae_debug("Found build dir: " << buildDir_.absolutePath());

	buildCmd_ = proj.value("buildcmd").toString();
	ae_debug("Found build command: " << buildCmd_);

	runCmd_ = proj.value("runcmd").toString();
	ae_debug("Found run command: " << runCmd_);

}

bool AeProject::parseCmakecache(const QString cacheFile) {
	QFile f(cacheFile);
	if(!f.open(QIODevice::ReadOnly))
		return false;
	QString contents = QString::fromLocal8Bit(f.readAll());
	QRegExp rx("CMAKE_PROJECT_NAME:STATIC=(\\w+)");
	if(rx.indexIn(contents) == -1)
		return false;
	projectName_ = rx.cap(1);
	ae_debug("Found project name: " << projectName_);

	rx = QRegExp(projectName_ + "_SOURCE_DIR:STATIC=([^\\n]+)");
	if(rx.indexIn(contents) == -1)
		return false;
	sourceDir_ = rx.cap(1);
	ae_debug("Found source dir: " << sourceDir_.absolutePath());

	rx = QRegExp(projectName_ + "_BINARY_DIR:STATIC=([^\\n]+)");
	if(rx.indexIn(contents) == -1)
		return false;
	buildDir_ = rx.cap(1);
	ae_debug("Found build dir: " << buildDir_.absolutePath());

	return true;
}

bool AeProject::containsPath(const QString path) const {
	return path.startsWith(sourceDir_.absolutePath()) || path.startsWith(buildDir_.absolutePath());
}

class UnboundTemporaryProject : public AeProject {
public:
	QString displayName() const { return "Unbound Project"; }
	QDir sourceDir() const { return QDir::currentPath(); }
	bool containsPath(const QString) const { return false; }
    bool projFileExists() const { return false; }
};

bool AeProject::projFileExists() const {
    QFileInfo fi(sourceDir_.absoluteFilePath(".aeproj"));
    return fi.exists();
}

void AeProject::writeProjectFile() {
    QSettings proj(sourceDir_.absoluteFilePath(".aeproj"), QSettings::IniFormat);
    proj.setValue("project", projectName_);
    proj.setValue("sourcedir", sourceDir_.absolutePath());
    ae_debug("builddir: "<<buildDir().absolutePath());
    proj.setValue("builddir", buildDir_.absolutePath());
    proj.setValue("buildcmd", buildCmd_);
    proj.setValue("runcmd", runCmd_);
    proj.sync();
}

AeProject* AeProject::getProject(const QList<AeProject*> projects, QString fileName) {
	// The filename may be empty if this is a new unsaved file
	if(fileName.isEmpty()) {
		ae_debug("Get project for empty file");
		return new UnboundTemporaryProject();
	}
	ae_debug("Get project for " << fileName);
	// The file may have a name but not yet be saved
	QFileInfo fi(fileName);
	if(!fi.exists()) {
		ae_debug("File does not exist");
		return new UnboundTemporaryProject();
	}
	// Alternatively, the file may belong to an existing project.
	// Search for it.
	QDir guessedSourceDir(fi.absoluteDir());
	foreach(AeProject* p, projects) {
		ae_debug("Comparing project " << p->displayName());
		if(p->containsPath(guessedSourceDir.absolutePath()))
			return p;
	}

	// No existing project suits. Attempt to open a new one.
	AeProject* p = new AeProject();
    // if nothing better comes up, this file's directory is the source dir and build dir
	p->setSourceDir(guessedSourceDir);
    p->buildDir_ = guessedSourceDir;
    // special case: this file is itself the project file, but no other editors from this project are open.
    if(fi.baseName() == ".aeproj") {
        ae_debug("getProject of .aeproj");
        p->parseAeproj(fileName);
        return p;
    }

	// first scan upwards looking for CMakeCache.txt
	QDir fileDir(guessedSourceDir);
	while(!fileDir.isRoot()) {
		ae_debug("Searching for CMakeCache.txt in " << fileDir.absolutePath());
		QFileInfo cmakecache(fileDir.absoluteFilePath("CMakeCache.txt"));
		if(cmakecache.exists()) {
			ae_debug("Found CMakeCache.txt in " << fileDir.absolutePath());
			if(p->parseCmakecache(cmakecache.filePath())) {
				guessedSourceDir = p->sourceDir();
				break;
			}
		}
		fileDir.cdUp();
	}

	// scan upwards looking for project files
	fileDir = guessedSourceDir;
	while(!fileDir.isRoot()) {
		ae_debug("Searching for .aeproj in " << fileDir.absolutePath());
		QFileInfo aeproj(fileDir.absoluteFilePath(".aeproj"));
		if(aeproj.exists()) {
			ae_debug("Found .aeproj in " << fileDir.absolutePath());
			p->parseAeproj(aeproj.filePath());
			return p;
		}
		fileDir.cdUp();
	}

    // if still not found, try CMakeLists. It's not enough to set project info, but it will give us a source dir
    fileDir = fi.absoluteDir();
    while(!fileDir.isRoot()) {
        QFileInfo cmakelists(fileDir.absoluteFilePath("CMakeLists.txt"));
        if(cmakelists.exists()) {
            ae_debug("Found CMakeLists.txt in " << fileDir.absolutePath());
            // TODO: potentially extract some info from this file
            p->setSourceDir(fileDir);
            break;
        }
        fileDir.cdUp();
    }

	// no project found. Assume the current dir.
	return p;
}
bool AeProject::build(QTextBrowser* displayWidget, bool andRun) {
    if(process_ && !process_->atEnd()) {
		int result = QMessageBox::question(0, "A process is already running", "Currently waiting for ``"+lastLaunchCmd_+"'' to complete execution. Do you want to kill it?", QMessageBox::Yes | QMessageBox::No);
		if(result == QMessageBox::Yes) {
			// todo properly
			process_->kill();
			if(!process_->waitForFinished(1000)) {
				ae_error("Could not kill process");
				return false;
			}
			// it was killed, continue
		} else {
			return false;
		}
	}
	process_ = new QProcess();
	outputWindow_ = displayWidget;
	connect(process_, SIGNAL(readyRead()), this, SLOT(processOutput()));
	connect(process_, SIGNAL(finished(int)), this, SLOT(processEnded(int)));
    connect(process_, SIGNAL(error(QProcess::ProcessError)), this, SLOT(processError(QProcess::ProcessError)));

	process_->setWorkingDirectory(buildDir_.absolutePath());
	outputWindow_->insertPlainText("Executing " + buildCmd_ + " from " + buildDir_.absolutePath() + "\n");
	lastLaunchCmd_ = buildCmd_;
	runOnExit_ = andRun;
	process_->start(lastLaunchCmd_);
	return true;
}

void AeProject::processOutput() {
	//outputWindow_->append(process_->readAll());
	outputWindow_->insertPlainText(process_->readAll());
}

void AeProject::processEnded(int exitCode) {
	outputWindow_->insertPlainText("Process " + lastLaunchCmd_ + " ended with exit code: " + QString::number(exitCode) + "\n");
	if(runOnExit_) {
		runOnExit_ = false;
		process_->setWorkingDirectory(buildDir_.absolutePath());
		outputWindow_->insertPlainText("Executing " + runCmd_ + " from " + buildDir_.absolutePath() + "\n");
		lastLaunchCmd_ = runCmd_;
		process_->start(lastLaunchCmd_);
	}
}

void AeProject::processError(QProcess::ProcessError e) {
    outputWindow_->insertPlainText("Error " + QString::number((int)e) + "\n");

}

