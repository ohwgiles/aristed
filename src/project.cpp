#include "project.hpp"

#include <QFileDialog>
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

void AeProject::parseAeproj(QFile projFile) {
	(void)projFile;
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
	ae_info("Found project name: " << projectName_);

	rx = QRegExp(projectName_ + "_SOURCE_DIR:STATIC=([^\\n]+)");
	if(rx.indexIn(contents) == -1)
		return false;
	sourceDir_ = rx.cap(1);
	ae_info("Found source dir: " << sourceDir_.absolutePath());

	rx = QRegExp(projectName_ + "_BINARY_DIR:STATIC=([^\\n]+)");
	if(rx.indexIn(contents) == -1)
		return false;
	buildDir_ = rx.cap(1);
	ae_info("Found build dir: " << buildDir_.absolutePath());

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
	ae_info("getProject for file " << fileName);
	if(fileName.isEmpty()) {
		ae_info("empty filename");
		return new UnboundTemporaryProject();
	}

	QFileInfo fi(fileName);
	if(!fi.exists()) {
		ae_info("File does not exist");
		return new UnboundTemporaryProject();
	}

	QDir fileDir(fi.absoluteDir());

	foreach(AeProject* p, projects) {
		if(p->containsPath(fileDir.absolutePath()))
			return p;
	}

	// first scan upwards looking for CMakeCache.txt
	while(!fileDir.isRoot()) {
		ae_info("Searching for CMakeCache.txt in " << fileDir.absolutePath());
		QFileInfo cmakecache(fileDir.absoluteFilePath("CMakeCache.txt"));

		if(cmakecache.exists()) {
			ae_info("Found CMakeCache.txt in " << fileDir.absolutePath());
			AeProject* p = new AeProject();
			if(p->parseCmakecache(cmakecache.filePath()))
			// todo scan source dir for .aeproj
			return p;
		}
		fileDir.cdUp();
	}

//	// scan upwards looking for project files
//	fileDir = fi.absoluteDir();
//	while(!fileDir.isRoot()) {
//		QFileInfo aeproj(fileDir.absoluteFilePath(".aeproj"));
//		if(aeproj.exists()) {
//			AeProject* p = new AeProject();
//			p->parseCmakelists(cmakecache.filePath());
//			return p;

//		}
//		fileDir.cdUp();
//	}

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
	AeProject* p = new AeProject();
	p->setSourceDir(fi.absoluteDir());
	return p;
}
