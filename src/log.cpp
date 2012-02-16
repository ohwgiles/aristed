#include "log.hpp"
#include <QString>
#include <iostream>

namespace {

const char* LogLevelToString(Log::LogLevel l) {
	switch(l) {
	case Log::TRACE: return "TRACE";
	case Log::DEBUG: return "DEBUG";
	case Log::INFO:  return "INFO";
	case Log::ERROR: return "ERROR";
	case Log::FATAL: return "FATAL";
	}
	return 0;
}

}

Log::~Log() {
	std::cout << "[" << LogLevelToString(lvl) << "] " << fn << ":" << line << " " << str() << std::endl;
}

std::ostream& operator<<(std::ostream& os, const QString& qs) {
	os << static_cast<const char*>(qs.toLocal8Bit().constData());
	return os;
}
