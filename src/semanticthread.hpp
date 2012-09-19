#ifndef SEMANTICTHREAD_HPP
#define SEMANTICTHREAD_HPP

#include <QObject>

class ThreadCaller;

class SemanticThread : public QObject {
	Q_OBJECT
public:
	SemanticThread(ThreadCaller& tc);
signals:
	void complete(char);
private slots:
	void trigger(char);
private:
	friend class ThreadCaller;
	ThreadCaller& tc;
};

class ThreadCaller {
public:
	ThreadCaller();
	virtual void doThreadWork(char) = 0;
	void triggerThread(char);
protected:
	SemanticThread t;
};

#endif // SEMANTICTHREAD_HPP
