#ifndef SEMANTICWORKER_HPP
#define SEMANTICWORKER_HPP

#include <QThread>
#include "callback.hpp"
#include "log.hpp"

namespace llvm { class MemoryBuffer; }

class ClangFile;

struct SemanticCallback {
	Callback<void, llvm::MemoryBuffer*, const char>::Arg2<ClangFile> callback;
	llvm::MemoryBuffer* buffer;
	char sentinel;
};

struct CompletionCallback {
	Callback<void, const char,int,int>::Arg3<ClangFile> callback;
	char sentinel;
	int line;
	int col;
};


// Silly little class to get around Qt's threading model/moc generation
class SemanticWorker : public QObject {
	Q_OBJECT
public:
	SemanticWorker() : mBusy(false) { thread.start(); moveToThread(&thread);}
	bool busy() const { return mBusy; }
protected:
	bool mBusy;
	QThread thread;
public slots:
	void handleSemanticWork(SemanticCallback cb) {
		mBusy = true;
		cb.callback(cb.buffer, cb.sentinel);
		mBusy = false;
	}
	void handleCompletionWork(CompletionCallback cb) {
		cb.callback(cb.sentinel, cb.line, cb.col);
	}
};

#endif // SEMANTICWORKER_HPP
