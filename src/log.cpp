#include "log.hpp"
#include <QString>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fstream>

#define COLOUR_TEAL    "\033[0;36m"
#define COLOUR_YELLOW  "\033[1;33m"
#define COLOUR_GREEN   "\033[0;32m"
#define COLOUR_RED     "\033[1;31m"
#define COLOUR_WHITE   "\033[1;37m"
#define COLOUR_NORMAL  "\033[m"

#define LOGSTR(loglevel) ( \
	loglevel==_Log::TRACE ? "TRACE" :\
	loglevel==_Log::DEBUG ? "DEBUG" :\
	loglevel==_Log::INFO  ? "INFO" :\
	loglevel==_Log::ERROR ? "ERROR" : 0)

#define COLOR(loglevel) ( \
	loglevel==_Log::TRACE ? COLOUR_TEAL :\
	loglevel==_Log::DEBUG ? COLOUR_YELLOW :\
	loglevel==_Log::INFO  ? COLOUR_GREEN :\
	loglevel==_Log::ERROR ? COLOUR_RED : 0)

namespace {

class LogFifo {
public:
	static std::fstream* stream;
	LogFifo() {
		unlink("/tmp/aristed.log");
		mkfifo("/tmp/aristed.log", 0600);
		stream = new std::fstream("/tmp/aristed.log");
	}
	~LogFifo() {
		delete stream;
	}
};

std::fstream* LogFifo::stream;
LogFifo __static_logfifo;

}

_Log::~_Log() {
#ifdef NDEBUG
	// only called for errors > ERROR
	std::cout << "Error: " << str() << std::endl;
#else
	(*LogFifo::stream) << "[" << COLOR(lvl) << LOGSTR(lvl) << COLOUR_NORMAL << "] " << fn << ":" << line << " " << str() << std::endl;
#endif
}

// handle QStrings
std::ostream& operator<<(std::ostream& os, const QString& qs) {
	os << static_cast<const char*>(qs.toLocal8Bit().constData());
	return os;
}
