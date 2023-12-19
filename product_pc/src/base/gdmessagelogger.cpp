/*************************************************
 *Copyright (C), 2019-2029, Guide Tech. Co., Ltd.
 *Project: IR236
 *File name: gdmessagelogger.cpp
 *Author: maolei
 *Email: maolei@guideir.com
 *Version:
 *Date: 2020-04-06
 *Description:
*************************************************/

#include "gdmessagelogger.h"

#include <Poco/AutoPtr.h>
#include <Poco/Util/PropertyFileConfiguration.h>
#include <Poco/Util/LoggingConfigurator.h>
#include <Poco/UnicodeConverter.h>
#include "Poco/Exception.h"
#include <iostream>
#include <QDir>
#include <QFileInfo>
#include <util.h>
#include <QCoreApplication>
#include <QDateTime>

#define gettid() syscall(__NR_gettid)
using namespace Poco;

void UTF8ToGBK(const std::string& utf8String, std::string& gbkString)
{
    std::wstring utf16String;
    std::string errorMessage;
    Poco::UnicodeConverter _unicodeConverter;
    try
    {
        _unicodeConverter.toUTF16(utf8String, utf16String);
        int size = 0;
        if(!gbkString.empty())
           gbkString.clear();
        //获得需要分配的空间大小
        size = WideCharToMultiByte(936, 0, utf16String.c_str(), -1, NULL, 0, NULL, NULL);
        std::vector<char> buff(size);
        if(WideCharToMultiByte(936, 0, utf16String.c_str(), -1, buff.data(), size, NULL, NULL) == 0)
        {
            gbkString.append(utf8String);
        }else {
            gbkString.append(buff.data(), buff.size());
        }
    }
    catch(Poco::Exception &e)
    {
        errorMessage.append("UTF8 convert to UTF16 failed, ");
        errorMessage.append(e.message());
    }

}

GdMessageLogger::GdMessageLogger(const std::string &logger_name, int log_level) :
    log_stream(new Poco::LogStream(logger_name, Poco::Message::Priority(log_level)))
{
}

GdMessageLogger::GdMessageLogger(const GdMessageLogger &other) :
    log_stream(other.log_stream),file(other.file),function(other.function),line(other.line)
{
    *log_stream << '[' << function << ' ' << line << "] ";

}

GdMessageLogger::~GdMessageLogger()
{
    flush();
}

void GdMessageLogger::init()
{
    Poco::AutoPtr<Poco::Util::PropertyFileConfiguration> pConf =  new Poco::Util::PropertyFileConfiguration();
    Poco::Util::LoggingConfigurator log_configurator;
    pConf->setString("logging.formatters.format_console.class","PatternFormatter");
    pConf->setString("logging.formatters.format_console.pattern","[%Y-%m-%d %H:%M:%S.%i]%t");
    pConf->setString("logging.formatters.format_console.times","local");
    pConf->setString("logging.formatters.format_file.class","PatternFormatter");
    pConf->setString("logging.formatters.format_file.pattern","[%Y-%m-%d %H:%M:%S.%i]%t");
    pConf->setString("logging.formatters.format_file.times","local");
    pConf->setString("logging.channels.console.class","ConsoleChannel");
    pConf->setString("logging.channels.console.formatter","format_console");
    pConf->setString("logging.channels.trace.class","FileChannel");
    pConf->setString("logging.channels.trace.formatter","format_file");
    pConf->setString("logging.channels.trace.rotation","50 M");
    pConf->setString("logging.channels.trace.archive","timestamp");
    pConf->setString("logging.channels.trace.times","local");
    pConf->setString("logging.channels.trace.compress","true");
    pConf->setString("logging.channels.trace.purgeAge","12 months");
    pConf->setString("logging.channels.trace.purgeCount","30");
    pConf->setString("logging.channels.trace.flush","false");
    pConf->setString("logging.channels.trace_splitter.class","SplitterChannel");
    pConf->setString("logging.channels.trace_splitter.channels","console,trace");
    pConf->setString("logging.loggers.trace.name","trace");
    pConf->setString("logging.loggers.trace.channel","console");
    pConf->setString("logging.loggers.debug.name","debug");
    pConf->setString("logging.loggers.debug.channel","trace_splitter");
    pConf->setString("logging.loggers.information.name","information");
    pConf->setString("logging.loggers.information.channel","trace_splitter");
    pConf->setString("logging.loggers.notice.name","notice");
    pConf->setString("logging.loggers.notice.channel","trace");
    pConf->setString("logging.loggers.warning.name","warning");
    pConf->setString("logging.loggers.warning.channel","trace_splitter");
    pConf->setString("logging.loggers.error.name","error");
    pConf->setString("logging.loggers.error.channel","trace_splitter");
    pConf->setString("logging.loggers.critical.name","critical");
    pConf->setString("logging.loggers.critical.channel","trace");

    QString sLogFilePath =QCoreApplication::applicationDirPath() + "\\logs\\"
            + QDateTime::currentDateTime().toString("yyyy_MM_dd")+"\\app_pc.log";
    QString sLogFileDir = QFileInfo(sLogFilePath).dir().path();
    if(!QDir::current().exists(sLogFileDir))
    {
        QDir::current().mkpath(sLogFileDir);
    }


    pConf->setString("logging.channels.trace.path",sLogFilePath.toStdString());
    pConf->setString("logging.loggers.app.level","debug");
    log_configurator.configure(pConf);
}

void GdMessageLogger::flush()
{
    *log_stream << std::endl;
}

std::mutex GdMessageLoggerHelper::mutex;

GdMessageLoggerHelper::GdMessageLoggerHelper(const char *file, int line, const char *function)
    : file(file), line(line)
{
    if(function)
    {
        size_t len = strlen(function);
        this->function = new char[len+1];
        this->function[len] = 0;
        memcpy(this->function, function, len);
    }
}

GdMessageLoggerHelper::~GdMessageLoggerHelper()
{
    mutex.unlock();
    if(function)
    {
        delete [] function;
    }
}

GdMessageLogger GdMessageLoggerHelper::trace()
{
    static GdMessageLogger *_trace = nullptr;
    if(!_trace)
    {
        mutex.lock();
        if(!_trace)
        {
            _trace = new GdMessageLogger("trace", Poco::Message::PRIO_TRACE);
        }
        mutex.unlock();
    }

    mutex.lock();
    _trace->file = file;
    _trace->function = function;
    _trace->line = line;
    return *_trace;
}

GdMessageLogger GdMessageLoggerHelper::debug()
{
    static GdMessageLogger *_debug = nullptr;
    if(!_debug)
    {
        mutex.lock();
        if(!_debug)
        {
            _debug = new GdMessageLogger("debug", Poco::Message::PRIO_DEBUG);
        }
        mutex.unlock();
    }

    mutex.lock();
    _debug->file = file;
    _debug->function = function;
    _debug->line = line;
    return *_debug;
}

GdMessageLogger GdMessageLoggerHelper::inform()
{
    static GdMessageLogger *_inform = nullptr;
    if(!_inform)
    {
        mutex.lock();
        if(!_inform)
        {
            _inform = new GdMessageLogger("information", Poco::Message::PRIO_INFORMATION);
        }
        mutex.unlock();
    }

    mutex.lock();
    _inform->file = file;
    _inform->function = function;
    _inform->line = line;
    return *_inform;
}

GdMessageLogger GdMessageLoggerHelper::notice()
{
    static GdMessageLogger *_notice = nullptr;
    if(!_notice)
    {
        mutex.lock();
        if(!_notice)
        {
            _notice = new GdMessageLogger("notice", Poco::Message::PRIO_NOTICE);
        }
        mutex.unlock();
    }

    mutex.lock();
    _notice->file = file;
    _notice->function = function;
    _notice->line = line;
    return *_notice;
}

GdMessageLogger GdMessageLoggerHelper::warning()
{
    static GdMessageLogger *_warning = nullptr;
    if(!_warning)
    {
        mutex.lock();
        if(!_warning)
        {
            _warning = new GdMessageLogger("warning", Poco::Message::PRIO_WARNING);
        }
        mutex.unlock();
    }

    mutex.lock();
    _warning->file = file;
    _warning->function = function;
    _warning->line = line;
    return *_warning;
}

GdMessageLogger GdMessageLoggerHelper::error()
{
    static GdMessageLogger *_error = nullptr;
    if(!_error)
    {
        mutex.lock();
        if(!_error)
        {
            _error = new GdMessageLogger("error", Poco::Message::PRIO_ERROR);
        }
        mutex.unlock();
    }

    mutex.lock();
    _error->file = file;
    _error->function = function;
    _error->line = line;
    return *_error;
}

GdMessageLogger GdMessageLoggerHelper::critical()
{
    static GdMessageLogger *_critical = nullptr;
    if(!_critical)
    {
        mutex.lock();
        if(!_critical)
        {
            _critical = new GdMessageLogger("critical", Poco::Message::PRIO_CRITICAL);
        }
        mutex.unlock();
    }

    mutex.lock();
    _critical->file = file;
    _critical->function = function;
    _critical->line = line;
    return *_critical;
}

//MyMessageLogger MyMessageLoggerHelper::fatal()
//{
//    //TODO: fatal的处理与其他不同
//}


#include <QtMessageHandler>
#include <QString>

/**
 * @brief LogMessageHandle 重定向回调函数
 * @param type
 * @param context
 * @param message
 */
void LogMessageHandle(QtMsgType type, const QMessageLogContext &context, const QString &message)
{
    const char *file = context.file ? context.file : "";
    QString strFunction(context.function);
    int index1 = strFunction.indexOf('(');
    int index2 = strFunction.lastIndexOf(' ', index1);
    if(index1 != -1 && index2 != -1)
    {
        strFunction = strFunction.mid(index2+1, index1-index2-1);
    }
    std::string sMsg = message.toStdString();
    std::string sOut ;
    UTF8ToGBK(sMsg, sOut);

    switch (type)
    {
    case QtDebugMsg:
        GdMessageLoggerHelper(file, context.line, strFunction.toStdString().c_str()).debug() << sOut;
        break;
    case QtInfoMsg:
        GdMessageLoggerHelper(file, context.line, strFunction.toStdString().c_str()).inform() << sOut;
        break;
    case QtWarningMsg:
        GdMessageLoggerHelper(file, context.line, strFunction.toStdString().c_str()).warning() << sOut;
        break;
    case QtCriticalMsg:
        GdMessageLoggerHelper(file, context.line, strFunction.toStdString().c_str()).critical() << sOut;
        break;
    case QtFatalMsg:
        //TODO: fatal的处理？
        //MyMessageLoggerHelper(context.file, context.line, context.function).fatal() << message.toStdString();
        break;
    }
}

void initGdMessageLogger()
{
    GdMessageLogger::init();
    qInstallMessageHandler(LogMessageHandle);
}

