#include "hm_logging.h"

#include <algorithm>
#include <time.h>
#include <iomanip>
#include <Windows.h>
#include <Strsafe.h>

#include "hm_lock.h"

namespace hm
{

const char* const log_severity_names[LS_LOG_NUM_SEVERITIES] = {
    "INFO", "WARNING", "ERROR", "ERROR_REPORT", "FATAL" };

LoggingDestination logging_destination = LOG_ONLY_TO_FILE;

t_string GetDefaultLogFile() {
    wchar_t module_name[MAX_PATH];
    GetModuleFileName(NULL, module_name, MAX_PATH);

    t_string log_file = module_name;
    size_t last_backslash =
        log_file.rfind('\\', log_file.size());
    if (last_backslash != t_string::npos)
        log_file.erase(last_backslash + 1);
    log_file += _T("debug.log");
    return log_file;
}

class LoggingLock {
public:
    LoggingLock() {
        LockLogging();
    }

    ~LoggingLock() {
        UnlockLogging();
    }

    static void Init(LogLockingState lock_log, const t_string &new_log_file) {
        if (initialized)
            return;
        lock_log_file = lock_log;
        if (lock_log_file == LOCK_LOG_FILE) {
            if (!log_mutex) {
                std::wstring safe_name;
                if (!new_log_file.empty())
                    safe_name = new_log_file;
                else
                    safe_name = GetDefaultLogFile();
                // \ is not a legal character in mutex names so we replace \ with /
                std::replace(safe_name.begin(), safe_name.end(), '\\', '/');
                std::wstring t(L"Global\\");
                t.append(safe_name);
                log_mutex = ::CreateMutex(NULL, FALSE, t.c_str());

                if (log_mutex == NULL) {
#if _DEBUG
                    // Keep the error code for debugging
                    int error = GetLastError();  // NOLINT
                    BreakDebugger();
#endif
                    // Return nicely without putting initialized to true.
                    return;
                }
            }
        } else {
            log_lock = new ThreadLock();
        }
        initialized = true;
    }

private:
    static void LockLogging() {
        if (lock_log_file == LOCK_LOG_FILE) {
            ::WaitForSingleObject(log_mutex, INFINITE);
        } else {
            // use the lock
            log_lock->Lock();
        }
    }

    static void UnlockLogging() {
        if (lock_log_file == LOCK_LOG_FILE) {
            ReleaseMutex(log_mutex);
        } else {
            log_lock->Unlock();
        }
    }

    // The lock is used if log file locking is false. It helps us avoid problems
    // with multiple threads writing to the log file at the same time.  Use
    // LockImpl directly instead of using Lock, because Lock makes logging calls.
    static ThreadLock* log_lock;

    // When we don't use a lock, we are using a global mutex. We need to do this
    // because LockFileEx is not thread safe.
    static HANDLE log_mutex;

    static bool initialized;
    static LogLockingState lock_log_file;
};

// static
bool LoggingLock::initialized = false;
// static
ThreadLock* LoggingLock::log_lock = NULL;
// static
LogLockingState LoggingLock::lock_log_file = LOCK_LOG_FILE;
// static
HANDLE LoggingLock::log_mutex = NULL;

bool log_process_id = false;
bool log_thread_id = false;
bool log_timestamp = true;
bool log_tickcount = false;

t_string* log_file_name = NULL;

// this file is lazily opened and the handle may be NULL
HANDLE log_file = NULL;

bool InitializeLogFileHandle() {
    if (log_file)
        return true;

    if (!log_file_name) {
        // Nobody has called InitLogging to specify a debug log file, so here we
        // initialize the log file name to a default.
        log_file_name = new t_string(GetDefaultLogFile());
    }

    if (logging_destination == LOG_ONLY_TO_FILE ||
        logging_destination == LOG_TO_BOTH_FILE_AND_SYSTEM_DEBUG_LOG) {
            log_file = CreateFile(log_file_name->c_str(), GENERIC_WRITE,
                FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
                OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
            if (log_file == INVALID_HANDLE_VALUE || log_file == NULL) {
                // try the current directory
                log_file = CreateFile(L".\\debug.log", GENERIC_WRITE,
                    FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
                    OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
                if (log_file == INVALID_HANDLE_VALUE || log_file == NULL) {
                    log_file = NULL;
                    return false;
                }
            }
            SetFilePointer(log_file, 0, 0, FILE_END);
    }

    return true;
}

LogMessage::LogMessage( const TCHAR* file, int line, LogSeverity severity, int ctr )
    : severity_(severity), file_(file), line_(line)
{
    Init(file, line);
}

LogMessage::LogMessage( const TCHAR* file, int line )
    : severity_(LS_LOG_INFO), file_(file), line_(line)
{
    Init(file, line);
}

LogMessage::LogMessage( const TCHAR* file, int line, LogSeverity severity )
    : severity_(severity), file_(file), line_(line)
{
    Init(file, line);
}

LogMessage::LogMessage( const TCHAR* file, int line, t_string* result )
    : severity_(LS_LOG_FATAL), file_(file), line_(line)
{
    Init(file, line);
    stream_ << _T("Check failed: ") << *result;
    delete result;
}

LogMessage::LogMessage( const TCHAR* file, int line, LogSeverity severity, t_string* result )
    : severity_(severity), file_(file), line_(line)
{
    Init(file, line);
    stream_ << _T("Check failed: ") << *result;
    delete result;
}

LogMessage::~LogMessage()
{
    stream_ << std::endl;
    t_string str_newline(stream_.str());

    if (logging_destination == LOG_ONLY_TO_SYSTEM_DEBUG_LOG ||
        logging_destination == LOG_TO_BOTH_FILE_AND_SYSTEM_DEBUG_LOG) {
        OutputDebugString(str_newline.c_str());
    }

    LoggingLock::Init(LOCK_LOG_FILE, _T(""));
    // write to log file
    if (logging_destination != LOG_NONE &&
        logging_destination != LOG_ONLY_TO_SYSTEM_DEBUG_LOG) {
            LoggingLock logging_lock;
            if (InitializeLogFileHandle()) {
                SetFilePointer(log_file, 0, 0, SEEK_END);
                DWORD num_written;
                WriteFile(log_file,
                    static_cast<const void*>(str_newline.c_str()),
                    static_cast<DWORD>(str_newline.length()*sizeof(TCHAR)),
                    &num_written,
                    NULL);
            }
    }
}

void LogMessage::Init( const TCHAR* file, int line )
{
    t_string filename(file);
    size_t last_slash_pos = filename.find_last_of(_T("\\/"));
    if (last_slash_pos != std::string::npos)
        filename.erase(0, last_slash_pos + 1);

    stream_ <<  _T('[');
    if (log_process_id)
        stream_ << ::GetCurrentProcessId() << _T(':');
    if (log_thread_id)
        stream_ << ::GetCurrentThreadId() << _T(':');
    if (log_timestamp) {
        time_t t = time(NULL);
        struct tm local_time = {0};
#if _MSC_VER >= 1400
        localtime_s(&local_time, &t);
#else
        localtime_r(&t, &local_time);
#endif
        struct tm* tm_time = &local_time;
        stream_ << std::setfill(_T('0'))
            << std::setw(2) << 1 + tm_time->tm_mon
            << std::setw(2) << tm_time->tm_mday
            << '/'
            << std::setw(2) << tm_time->tm_hour
            << std::setw(2) << tm_time->tm_min
            << std::setw(2) << tm_time->tm_sec
            << ':';
    }
    if (log_tickcount)
        stream_ << GetTickCount() << _T(':');
    if (severity_ >= 0)
        stream_ << log_severity_names[severity_];
    else
        stream_ << _T("VERBOSE") << -severity_;

    stream_ << _T(":") << filename << _T("(") << line << _T(")] ");

    message_start_ = stream_.tellp();
}

void LogMessage::FormatLog( const TCHAR* fmt, ... )
{
    const size_t max_log_len = 1024;
    va_list arg_list;
    TCHAR buffer[max_log_len] = {0};

    va_start(arg_list, fmt);

    if (!SUCCEEDED(StringCchVPrintf(buffer, max_log_len, fmt, arg_list))) {
        va_end(arg_list);
        return;
    }
    stream() << buffer;

    va_end(arg_list);
}


LogMessage::SaveLastError::SaveLastError()
    : last_error_(::GetLastError())
{
}

LogMessage::SaveLastError::~SaveLastError()
{
    ::SetLastError(last_error_);
}

bool InitLogging( const t_string &new_log_file, LoggingDestination logging_dest, LogLockingState lock_log, OldFileDeletionState delete_old )
{
    LoggingLock::Init(lock_log, new_log_file);

    LoggingLock logging_lock;

    if (log_file) {
        // calling InitLogging twice or after some log call has already opened the
        // default log file will re-initialize to the new options
        ::CloseHandle(log_file);
        log_file = NULL;
    }

    logging_destination = logging_dest;

    // ignore file options if logging is disabled or only to system
    if (logging_destination == LOG_NONE ||
        logging_destination == LOG_ONLY_TO_SYSTEM_DEBUG_LOG)
        return true;

    if (!log_file_name)
        log_file_name = new t_string();
    *log_file_name = new_log_file;
    if (delete_old == DELETE_OLD_LOG_FILE)
        ::DeleteFile(log_file_name->c_str());

    return InitializeLogFileHandle();
}

void CloseLogFile()
{
    LoggingLock logging_lock;

    if (!log_file)
        return;

    CloseHandle(log_file);
    log_file = NULL;
}

}

