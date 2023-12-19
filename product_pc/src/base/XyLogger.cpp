#include "XyLogger.h"

XyLogger::XyLogger(std::string name, std::string filepath) : m_name(name),
    m_category(log4cpp::Category::getInstance(name))
{
    m_appender = new log4cpp::FileAppender(name, filepath);
    m_pattern  = new log4cpp::PatternLayout();
    m_pattern->setConversionPattern("%d{%Y-%m-%d %H:%M:%S.%l}[%t][%c][%p] %m %n");

    m_appender->setLayout(m_pattern);
    m_category.addAppender(m_appender);
}

XyLogger::~XyLogger()
{
    if (m_appender) {
        delete m_appender;
        m_appender = nullptr;
    }

    if (m_pattern) {
        delete m_pattern;
        m_pattern = nullptr;
    }
}

void XyLogger::infor(std::string log)
{
    m_category << log4cpp::Priority::INFO << log;
}

void XyLogger::warning(std::string log)
{
    m_category << log4cpp::Priority::WARN << log;
}

void XyLogger::error(std::string log)
{
    m_category << log4cpp::Priority::ERROR << log;
}
