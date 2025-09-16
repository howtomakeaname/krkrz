#include "tjsCommHead.h"
#include "CharacterSet.h"
#include "LogIntf.h"

#include <plog/Log.h>
#include <plog/Init.h>
#include <plog/Formatters/TxtFormatter.h>

// TJSログレベルから plog のログレベルに変換する関数
static plog::Severity TVPLogLevelToPlogSeverity(TVPLogLevel logLevel) 
{
    switch (logLevel) {
        case TVPLOG_LEVEL_VERBOSE:
            return plog::verbose;
        case TVPLOG_LEVEL_DEBUG:
            return plog::debug;
        case TVPLOG_LEVEL_INFO:
            return plog::info;
        case TVPLOG_LEVEL_WARNING:
            return plog::warning;
        case TVPLOG_LEVEL_ERROR:
            return plog::error;
        case TVPLOG_LEVEL_CRITICAL:
            return plog::fatal;
        default:
            return plog::none;
    }
}

void TVPLogSetLevel(TVPLogLevel logLevel)
{
    auto logger = plog::get();
    if (logger) {
        logger->setMaxSeverity(TVPLogLevelToPlogSeverity(logLevel));
    }   
}

void TVPLog(TVPLogLevel logLevel, const char *file, int line, const char *func, const char *format, fmt::format_args args)
{
    auto logger = plog::get();
    if (logger) {
        plog::Record record(TVPLogLevelToPlogSeverity(logLevel), func, line, file, 0, 0);
        std::string msg;
        try {
            // fmt::vformat は例外を投げる可能性があるので、try-catchで囲む
            msg = fmt::vformat(format, args);
        } catch (const fmt::format_error& e) {
            msg = "Log Format error: " + std::string(e.what());
        }
        record << fmt::vformat(format, args);
        logger->write(record.ref());
    }
}

void TVPLogMsg(TVPLogLevel logLevel, const char *msg)
{
    auto logger = plog::get();
    if (logger) {
        plog::Record record(TVPLogLevelToPlogSeverity(logLevel), "", 0, "", 0, 0);
        record << msg;
        logger->write(record.ref());
    }
}

#ifdef _WIN32

//---------------------------------------------------------------------------
// plog の標準の ConsoleAdapter は msys で問題があるので別実装
//---------------------------------------------------------------------------

#include <plog/Appenders/IAppender.h>
#include <plog/Util.h>
#include <plog/WinApi.h>

class WinConsoleAppender : public plog::IAppender
{
public:
    WinConsoleAppender() {}

    virtual void write(const plog::Record& record) override
    {
        plog::util::nstring str = plog::TxtFormatter::format(record);
        plog::util::MutexLock lock(m_mutex);

        const std::wstring& wstr = plog::util::toWide(str);
        const tjs_char *mes = (tjs_char*)wstr.c_str();
        tjs_int len = wstr.size();

        HANDLE hStdOutput = ::GetStdHandle(STD_OUTPUT_HANDLE);
        if (hStdOutput != INVALID_HANDLE_VALUE) {
            DWORD mode;
            if (GetConsoleMode(hStdOutput, &mode)) {
                ::WriteConsoleW(hStdOutput, mes, len, NULL, NULL);        
            } else {
                // その他のハンドル, UTF-8で出力
                static std::vector<char> console_cache_(200);
                tjs_int u8len = TVPWideCharToUtf8String( mes, len, nullptr );
                if (console_cache_.size() < u8len ) {
                    console_cache_.resize(u8len);
                }
                TVPWideCharToUtf8String( mes, len, &(console_cache_[0]) );
                ::WriteFile( hStdOutput, &(console_cache_[0]), u8len, NULL, NULL );
            }
        }
    }
protected:
    plog::util::Mutex m_mutex;
};

void TVPLogInit(TVPLogLevel logLevel) 
{
    static WinConsoleAppender consoleAppender;
    plog::init(TVPLogLevelToPlogSeverity(logLevel), &consoleAppender);
}

#else

#include <plog/Appenders/ColorConsoleAppender.h>
void TVPLogInit(TVPLogLevel logLevel) 
{
    // コンソールアペンダーを初期化
    static plog::ColorConsoleAppender<plog::TxtFormatter> consoleAppender(plog::streamStdErr);
    plog::init(TVPLogLevelToPlogSeverity(logLevel), &consoleAppender);
}

#endif