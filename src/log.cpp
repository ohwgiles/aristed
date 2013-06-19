/* Copyright 2013 Oliver Giles
 * This file is part of Aristed. Aristed is licensed under the 
 * GNU GPL version 3. See LICENSE or <http://www.gnu.org/licenses/>
 * for more information */
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


_Log::~_Log() {
#ifdef NDEBUG
	// only called for errors > ERROR
	std::cout << "Error: " << str() << std::endl;
#else
    std::cout << "[" << COLOR(lvl) << LOGSTR(lvl) << COLOUR_NORMAL << "] " << fn << ":" << line << " " << str() << std::endl;
#endif
}

// handle QStrings
std::ostream& operator<<(std::ostream& os, const QString& qs) {
	os << static_cast<const char*>(qs.toLocal8Bit().constData());
	return os;
}
