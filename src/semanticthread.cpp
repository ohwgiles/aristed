#include "semanticthread.hpp"

#include <QThread>
#include <QMetaMethod>
namespace {

static bool thread_started = false;
static QThread background_thread;

}

void ThreadCaller::triggerThread(char s) {
	// This class exists in the caller's thread. Pass to the background thread
	// via QMetaMethod
	QMetaObject::invokeMethod(&t,"trigger", Q_ARG(char, s));
}

ThreadCaller::ThreadCaller() : t(*this) {
}

SemanticThread::SemanticThread(ThreadCaller &tc) :
	QObject(),
	tc(tc)
{
	if(thread_started == false) {
		background_thread.start();
		thread_started = true;
	}
	moveToThread(&background_thread);
}

void SemanticThread::trigger(char x) {
	tc.doThreadWork(x);
	emit complete(x);
}
