#ifndef HM_LOGGING_H_H
#define HM_LOGGING_H_H

#include <ostream>
#include <sstream>

#include "hm_common.h"

namespace hm
{

enum LoggingDestination { LOG_NONE,
    LOG_ONLY_TO_FILE,
    LOG_ONLY_TO_SYSTEM_DEBUG_LOG,
    LOG_TO_BOTH_FILE_AND_SYSTEM_DEBUG_LOG };

enum LogLockingState { LOCK_LOG_FILE, DONT_LOCK_LOG_FILE };

// On startup, should we delete or append to an existing log file (if any)?
// Defaults to APPEND_TO_OLD_LOG_FILE.
enum OldFileDeletionState { DELETE_OLD_LOG_FILE, APPEND_TO_OLD_LOG_FILE };

typedef int LogSeverity;
const LogSeverity LOG_VERBOSE = -1;  // This is level 1 verbosity
// Note: the log severities are used to index into the array of names,
// see log_severity_names.
const LogSeverity LS_LOG_INFO = 0;
const LogSeverity LS_LOG_WARNING = 1;
const LogSeverity LS_LOG_ERROR = 2;
const LogSeverity LS_LOG_ERROR_REPORT = 3;
const LogSeverity LS_LOG_FATAL = 4;
const LogSeverity LS_LOG_NUM_SEVERITIES = 5;

bool InitLogging(const t_string &new_log_file,
    LoggingDestination logging_dest,
    LogLockingState lock_log,
    OldFileDeletionState delete_old);
void CloseLogFile();

#ifdef _UNICODE
typedef std::wostringstream OutStream;
#else
typedef std::ostringstream OutStream;
#endif

class LogMessage
{
public:
    LogMessage(const TCHAR* file, int line, LogSeverity severity, int ctr);

    LogMessage(const TCHAR* file, int line);

    LogMessage(const TCHAR* file, int line, LogSeverity severity);

    LogMessage(const TCHAR* file, int line, t_string* result);

    LogMessage(const TCHAR* file, int line, LogSeverity severity,
        t_string* result);

    ~LogMessage();

    OutStream& stream() { return stream_; }

    void FormatLog(const TCHAR* fmt, ...);

private:
    void Init(const TCHAR* file, int line);

    LogSeverity severity_;
    OutStream stream_;
    std::streamoff message_start_;  // Offset of the start of the message (past prefix
    // info).
    // The file and line information passed in to the constructor.
    const TCHAR* file_;
    const int line_;

    // Stores the current value of GetLastError in the constructor and restores
    // it in the destructor by calling SetLastError.
    // This is useful since the LogMessage class uses a lot of Win32 calls
    // that will lose the value of GLE and the code that called the log function
    // will have lost the thread error value when the log call returns.
    class SaveLastError {
    public:
        SaveLastError();
        ~SaveLastError();

        unsigned long get_error() const { return last_error_; }

    protected:
        unsigned long last_error_;
    };

    SaveLastError last_error_;

    DISALLOW_COPY_AND_ASSIGN(LogMessage);
};

#define LOG_INFO(...) \
    hm::LogMessage(_T(__FILE__), __LINE__, hm::LS_LOG_INFO).FormatLog(##__VA_ARGS__)
#define LOG_WARNING(...) \
    hm::LogMessage(_T(__FILE__), __LINE__, hm::LS_LOG_WARNING).FormatLog(##__VA_ARGS__)
#define LOG_ERROR(...) \
    hm::LogMessage(_T(__FILE__), __LINE__, hm::LS_LOG_ERROR).FormatLog(##__VA_ARGS__)
#define LOG_ERROR_REPORT(...) \
    hm::LogMessage(_T(__FILE__), __LINE__, \
    hm::LOG_ERROR_REPORT).FormatLog(##__VA_ARGS__)
#define LOG_FATAL(...) \
    hm::LogMessage(_T(__FILE__), __LINE__, hm::LS_LOG_FATAL).FormatLog(##__VA_ARGS__)
}

#endif
