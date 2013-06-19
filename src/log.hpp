#ifndef Log_HPP
#define Log_HPP
/* Copyright 2013 Oliver Giles
 * This file is part of Aristed. Aristed is licensed under the 
 * GNU GPL version 3. See LICENSE or <http://www.gnu.org/licenses/>
 * for more information */
#include <sstream>
#include <ostream>
#include <cstdlib>

class _Log : public std::stringstream {
public:
	enum LogLevel { TRACE, DEBUG, INFO, ERROR, FATAL };
	_Log(LogLevel level, const char* fn, int line)
		: lvl(level), fn(fn), line(line) {}
	virtual ~_Log();

	std::stringstream& stream() { return *this; }

private:
	LogLevel lvl;
	const char* fn;
	int line;
};

class QString;
std::ostream& operator<<(std::ostream& os, const QString& qs);

#define  ae_fatal(msg) { _Log(_Log::ERROR, __PRETTY_FUNCTION__, __LINE__).stream() << msg; exit(1); }
#define  ae_error(msg) _Log(_Log::ERROR, __PRETTY_FUNCTION__, __LINE__).stream() << msg

#ifdef NDEBUG
#define   ae_info(msg)
#define  ae_debug(msg)
#define  ae_trace(msg)
#define ae_assert(cond)
#else
#define   ae_info(msg) _Log(_Log::INFO , __PRETTY_FUNCTION__, __LINE__).stream() << msg
#define  ae_debug(msg) _Log(_Log::DEBUG, __PRETTY_FUNCTION__, __LINE__).stream() << msg
#define  ae_trace(msg) _Log(_Log::TRACE, __PRETTY_FUNCTION__, __LINE__).stream() << msg
#define ae_assert(cond) if(!(cond)) { ae_fatal("Assertion " # cond " failed "); }
#endif

#endif // Log_HPP
