#ifndef PROJECT_HPP
#define PROJECT_HPP

#include <QDir>
#include <QList>
#include <QFile>

class AeEditor;

class AeProject {
public:
	AeProject();
	virtual ~AeProject() {}

	virtual QDir sourceDir() const { return sourceDir_; }
	QDir buildDir() const { return buildDir_; }

	virtual QString displayName() const;

	QList<AeEditor*> editors_;

	static AeProject* getProject(const QList<AeProject *> projects, QString fileName);

private:
	void setSourceDir(QDir sourceDir);
	void parseAeproj(const QString projFile);
	bool parseCmakecache(const QString cacheFile);

	virtual bool containsPath(const QString path) const;

	QDir sourceDir_;
	QDir buildDir_;
	QString buildCmd_;
	QString projectName_;
};


#endif // PROJECT_HPP
