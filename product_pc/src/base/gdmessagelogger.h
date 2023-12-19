/*************************************************
 *Copyright (C), 2019-2029, Guide Tech. Co., Ltd.
 *Project: IR236
 *File name: gdmessagelogger.h
 *Author: maolei
 *Email: maolei@guideir.com
 *Version:
 *Date: 2020-04-06
 *Description:
 *GdMessageLogger是日志输出库，底层基于Poco::LogStream
 * 基本用法: 1. 运行目录中，log_conf.properties是日志系统配置文件，根据配置确定各级别日志的格式和参数等
 *          2. 流式输出，支持标准C++的可输出数据类型，输出结束自动换行。使用示例：
 *             GdTrace()   << 123;
 *             GdDebug()   << 456.78;
 *             GdInform()  << true << false;
 *             GdNotice()  << string;
 *             GdWarning() << string << 123 << 456.78;
 *             GdError()   << std::endl;
 *          3. initGdMessageLogger()是初始化函数，其中包含重定向功能，
 *             将QDebug重定向到GdMessageLogger进行输出，因此支持所有Qt的可输出数据类型
*************************************************/

#ifndef MYMESSAGELOGGER_H
#define MYMESSAGELOGGER_H

#include <Poco/LogStream.h>
#include <mutex>

class GdMessageLogger
{
protected:
    GdMessageLogger(const std::string &logger_name, int log_level);
    GdMessageLogger(const GdMessageLogger &other);

public:
    ~GdMessageLogger();

    static void init();

    template<class T>
    GdMessageLogger &operator <<(const T &t)
    {
        *log_stream << t;
        *log_stream << "  ";
        return *this;
    }

    GdMessageLogger &operator <<(std::ostream& (*op)(std::ostream&))
    {
        *log_stream << op;
        return *this;
    }

    void flush();

private:
    Poco::LogStream *log_stream;
    const char *file;
    const char *function;
    int line;

    friend class GdMessageLoggerHelper;
};

class GdMessageLoggerHelper
{
public:
    GdMessageLoggerHelper(const char *file, int line, const char *function);

    ~GdMessageLoggerHelper();

    GdMessageLogger trace();
    GdMessageLogger debug();
    GdMessageLogger inform();
    GdMessageLogger notice();
    GdMessageLogger warning();
    GdMessageLogger error();
    GdMessageLogger critical();
//    MyMessageLogger fatal();

private:
    const char *file;
    int line;
    char *function = nullptr;
    static std::mutex mutex;
};

#define GdTrace    GdMessageLoggerHelper(__FILE__, __LINE__, __FUNCTION__).trace
#define GdDebug    GdMessageLoggerHelper(__FILE__, __LINE__, __FUNCTION__).debug
#define GdInform   GdMessageLoggerHelper(__FILE__, __LINE__, __FUNCTION__).inform
#define GdNotice   GdMessageLoggerHelper(__FILE__, __LINE__, __FUNCTION__).notice
#define GdWarning  GdMessageLoggerHelper(__FILE__, __LINE__, __FUNCTION__).warning
#define GdError    GdMessageLoggerHelper(__FILE__, __LINE__, __FUNCTION__).error
#define GdCritical GdMessageLoggerHelper(__FILE__, __LINE__, __FUNCTION__).critical

/**
 * @brief initGdMessageLogger 初始化日志系统，重定向QDebug的输出信息
 */
void initGdMessageLogger();

#endif // MYMESSAGELOGGER_H
