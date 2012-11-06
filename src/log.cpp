#include "log.hpp"
#include <QString>
#include <iostream>

namespace {

const char* LogLevelToString(_Log::LogLevel l) {
	switch(l) {
	case _Log::TRACE: return "TRACE";
	case _Log::DEBUG: return "DEBUG";
	case _Log::INFO:  return "INFO";
	case _Log::ERROR: return "ERROR";
	case _Log::FATAL: return "FATAL";
	}
	return 0;
}

}

_Log::~_Log() {
	std::cout << "[" << LogLevelToString(lvl) << "] " << fn << ":" << line << " " << str() << std::endl;
}

std::ostream& operator<<(std::ostream& os, const QString& qs) {
	os << static_cast<const char*>(qs.toLocal8Bit().constData());
	return os;
}
