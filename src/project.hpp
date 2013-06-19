#ifndef PROJECT_HPP
#define PROJECT_HPP
/* Copyright 2013 Oliver Giles
 * This file is part of Aristed. Aristed is licensed under the 
 * GNU GPL version 3. See LICENSE or <http://www.gnu.org/licenses/>
 * for more information */
#include <QDir>
#include <QList>
#include <QFile>
#include <QProcess>

class AeEditor;
class QProcess;
class QTextBrowser;

class AeProject : public QObject {
	Q_OBJECT
public:
	AeProject();
	virtual ~AeProject() {}

	virtual QDir sourceDir() const { return sourceDir_; }
	QDir buildDir() const { return buildDir_; }

	virtual QString displayName() const;

	QList<AeEditor*> editors_;

	static AeProject* getProject(const QList<AeProject *> projects, QString fileName);

	bool build(QTextBrowser* displayWidget, bool andRun);

    virtual bool projFileExists() const;

    void setSourceDir(QDir sourceDir);

    void writeProjectFile();
    void parseAeproj(const QString projFile);
private:
	bool parseCmakecache(const QString cacheFile);

	virtual bool containsPath(const QString path) const;

	QDir sourceDir_;
	QDir buildDir_;
	QString buildCmd_;
	QString runCmd_;
	QString projectName_;
	QString lastLaunchCmd_;
	QProcess* process_;
	QTextBrowser* outputWindow_;
	bool runOnExit_;
private slots:
	void processOutput();
	void processEnded(int exitCode);
    void processError(QProcess::ProcessError e);
};


#endif // PROJECT_HPP
