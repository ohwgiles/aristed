#include "project.hpp"

#include <QFileDialog>

AeProject::AeProject()
{
}
QString AeProject::displayName() const {
	return "??";
}
void AeProject::setSourceDir(QDir sourceDir) {
	sourceDir_ = sourceDir;
}

void AeProject::parseAeproj(QFile projFile) {
	(void)projFile;
}

void AeProject::parseCmakelists(QFile cmakeFile) {
	(void)cmakeFile;
}


class UnboundTemporaryProject : public AeProject {
public:
	QString displayName() const { return "Unbound Project"; }
};

AeProject* AeProject::getProject(const QList<AeProject*> projects, QString fileName) {
	if(fileName.isEmpty())
		return new UnboundTemporaryProject();

	QFileInfo fi(fileName);
	if(!fi.exists())
		return new UnboundTemporaryProject();

	QDir fileDir(fi.absoluteDir());

	foreach(AeProject* p, projects) {
		if(fileDir.absolutePath().startsWith(p->sourceDir().absolutePath()) ||
				fileDir.absolutePath().startsWith(p->buildDir().absolutePath()))
			return p;
	}

//	// first scan upwards looking for CMakeCache.txt
//	while(!fileDir.isRoot()) {
//		QFileInfo cmakecache(fileDir.absoluteFilePath("CMakeCache.txt"));
//		if(cmakecache.exists()) {
//			AeProject* p = new AeProject();
//			p->parseCmakelists(cmakecache.filePath());
//			// todo scan source dir for .aeproj
//			return p;
//		}
//		fileDir.cdUp();
//	}

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
