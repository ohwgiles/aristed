#include "project.hpp"

#include <QFileDialog>
#include <QSettings>

#include "log.hpp"
AeProject::AeProject()
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

};

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
	// if nothing better comes up, this file's directory is the source dir
	p->setSourceDir(guessedSourceDir);

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

//	// if still not found, try CMakeLists and prompt for .aeproj creation
//	fileDir = fi.absoluteDir();
//	while(!fileDir.isRoot()) {
//		QFileInfo cmakelists(fileDir.absoluteFilePath("CMakeLists.txt"));
//		if(cmakelists.exists()) {
//			QString buildDir = QFileDialog::getExistingDirectory(0, "A CMakeLists.txt file was found. Please select the directory containing CMakeCache.txt to help aristed index code", fileDir);
//			if(!buildDir.isEmpty()) {
//				// todo check cmakecache actually exists here
//				QDir buildPath(buildDir);
//				AeProject* p = new AeProject();
//				p->parseCmakelists(buildPath.absoluteFilePath("CMakeCache.txt"));
//				// todo ask the user to store changes in .aeproj
//				return p;
//			}
//		}
//		fileDir.cdUp();
//	}

	// no project found. Assume the current dir.
	return p;
}
