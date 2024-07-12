#pragma once
#include <string>
#include <memory>
#include <exception>
#include <QVariant>
class CYamlParserException : public std::exception
{
public:
    CYamlParserException(const std::string &msg) : std::exception(msg.c_str()) {}
    const char* what() const throw() { return m_msg.c_str(); }

private:
    std::string m_msg;
};

class CYamlParser
{
public:
    CYamlParser();
    ~CYamlParser();

    bool loadFromFile(const std::string &filename);
    bool save();

    template <typename T>
    T get(const std::string& key, const std::string& section = "", const T &defaultValue = T());
    QVariant get(const std::string& key, const std::string& section = "", const QVariant &defaultValue = QVariant());

    template <typename T>
    bool set(const std::string& key, const T& value, const std::string& section = "");
    bool set(const std::string& key, const QVariant& value, const std::string& section = "");

private:
    struct Impl;
    std::unique_ptr<Impl> m_Impl;

};

