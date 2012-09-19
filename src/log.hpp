#ifndef LOG_HPP
#define LOG_HPP

#include <sstream>
#include <ostream>
#include <cstdlib>
class Log : public std::stringstream
{
public:
    enum LogLevel { TRACE, DEBUG, INFO, ERROR, FATAL };
	Log(LogLevel level, const char* fn, int line)
		: lvl(level), fn(fn), line(line)
	{}
    virtual ~Log();
    std::stringstream& stream() { return *this; }
private:
    LogLevel lvl;
    const char* fn;
    int line;
};

class QString;
std::ostream& operator<<(std::ostream& os, const QString& qs);

#define  ae_fatal(msg) { Log(Log::ERROR, __PRETTY_FUNCTION__, __LINE__).stream() << msg; exit(1); }
#define  ae_error(msg) Log(Log::ERROR, __PRETTY_FUNCTION__, __LINE__).stream() << msg
#define   ae_info(msg) Log(Log::INFO , __PRETTY_FUNCTION__, __LINE__).stream() << msg
#define  ae_debug(msg) Log(Log::DEBUG, __PRETTY_FUNCTION__, __LINE__).stream() << msg
#define  ae_trace(msg) Log(Log::TRACE, __PRETTY_FUNCTION__, __LINE__).stream() << msg
#define ae_assert(cond) if(!(cond)) { ae_fatal("Assertion " # cond " failed "); }

#endif // LOG_HPP
