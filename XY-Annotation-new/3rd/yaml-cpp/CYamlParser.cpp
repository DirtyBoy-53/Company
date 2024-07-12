#include "CYamlParser.h"
#include <fstream>
#include <QDebug>
#include "yaml-cpp/yaml.h"

struct CYamlParser::Impl{
    std::string m_filename;
    bool m_isLoad{false};
    YAML::Node m_node;
    template<typename T>
    bool write(const std::string &key, const T &value, const std::string &section){
        if (!m_isLoad) return false;

        if (section.empty()) {
            m_node[key] = value;
            return true;
        }
        else {
            m_node[section][key] = value;
            return true;
        }
    }
    template<typename T>
    T read(const std::string& key, const std::string& section, T defaultValue){

        T value = T();
        try {
            if (section.empty()) {
                value = m_node[key].as<T>();
            }
            else {
                value = m_node[section][key].as<T>();
            }
        }
        catch (const std::exception& e) {
            qWarning() << e.what();
            if(strcmp(e.what(),"bad conversion") == 0){
                if(value == T()){
                    write(key, defaultValue, section);
                    value = defaultValue;
                }
            }
        }
        return value;
    }
};

CYamlParser::CYamlParser()
    : m_Impl(std::make_unique<Impl>())
{
}

CYamlParser::~CYamlParser()
{
    save();
    m_Impl->m_isLoad = false;
}

bool CYamlParser::loadFromFile(const std::string& filename)
{
    m_Impl->m_filename = filename;
    try {
        m_Impl->m_node = YAML::LoadFile(filename);
        m_Impl->m_isLoad = true;
    }
    catch (const std::exception& e) {
        qWarning() << e.what();
        return false;
    }
    return false;
}

bool CYamlParser::save()
{
    if (!m_Impl->m_isLoad) return false;

    if(m_Impl->m_filename.empty())
        throw CYamlParserException("filename is empty");
    try {
        std::ofstream out(m_Impl->m_filename);
        out << m_Impl->m_node;
    }
    catch (const std::exception& e) {
        qWarning() << e.what();
        return false;
    }
    m_Impl->m_isLoad = false;
    return true;
}

/*
 * Use QVariant as value
 */
QVariant CYamlParser::get(const std::string &key, const std::string &section, const QVariant &defaultValue)
{
    std::string ret = m_Impl->read<std::string>(key, section, defaultValue.toString().toStdString());
    return QVariant(QString::fromStdString(ret));
}

bool CYamlParser::set(const std::string &key, const QVariant &value, const std::string &section)
{
    return m_Impl->write<std::string>(key, value.toString().toStdString(), section);
}



/*
 * get
 * Passing values ​​using member function templates
 */
template<>
std::string CYamlParser::get(const std::string& key, const std::string& section, const std::string &defaultValue)
{
    return m_Impl->read<std::string>(key, section, defaultValue);
}

template<>
QString CYamlParser::get(const std::string& key, const std::string& section, const QString &defaultValue)
{
    std::string ret = m_Impl->read<std::string>(key, section, defaultValue.toStdString());
    return QString::fromStdString(ret);
}

template<>
int CYamlParser::get(const std::string& key, const std::string& section, const int &defaultValue)
{
    return m_Impl->read<int>(key, section, defaultValue);
}

template<>
double CYamlParser::get(const std::string& key, const std::string& section, const double &defaultValue)
{
    return m_Impl->read<double>(key, section, defaultValue);
}

template<>
float CYamlParser::get(const std::string& key, const std::string& section, const float &defaultValue)
{
    return m_Impl->read<float>(key, section, defaultValue);
}


/*
 * set
 * Passing values ​​using member function templates
 */

template<>
bool CYamlParser::set(const std::string &key, const std::string &value, const std::string &section)
{
    return m_Impl->write<std::string>(key, value, section);
}

template<>
bool CYamlParser::set(const std::string &key, const QString &value, const std::string &section)
{
    return m_Impl->write<std::string>(key, value.toStdString(), section);
}

template<>
bool CYamlParser::set(const std::string &key, const int &value, const std::string &section)
{
    return m_Impl->write<int>(key, value, section);
}

template<>
bool CYamlParser::set(const std::string &key, const float &value, const std::string &section)
{
    return m_Impl->write<float>(key, value, section);
}

template<>
bool CYamlParser::set(const std::string &key, const double &value, const std::string &section)
{
    return m_Impl->write<double>(key, value, section);
}
