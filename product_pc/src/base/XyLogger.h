#ifndef XYLOGGER_H
#define XYLOGGER_H

#include <string>
#include <memory>

#include "log4cpp/FileAppender.hh"
#include "log4cpp/Category.hh"
#include "log4cpp/PatternLayout.hh"

class XyLogger
{
public:
    XyLogger(std::string name = "log", std::string filepath  = "D:/log.log");
    ~XyLogger();
    void infor(std::string log);
    void warning(std::string log);
    void error(std::string log);

private:
    std::string m_name;
    log4cpp::Category & m_category;
    log4cpp::Appender *m_appender;
    log4cpp::PatternLayout *m_pattern;
};

#endif // XYLOGGER_H
