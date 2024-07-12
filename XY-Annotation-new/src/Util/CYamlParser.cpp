#include "CYamlParser.h"
#include <fstream>
#include <QDebug>
#include "yaml-cpp/yaml.h"
using namespace YAML;

struct CYamlParser::Impl{
	std::string m_filename;
	bool m_isLoad{false};
	//YAML::Node m_node;
};

CYamlParser::CYamlParser()
	: m_pImpl(std::make_unique<Impl>())
{
}

CYamlParser::~CYamlParser()
{
}

bool CYamlParser::loadFromFile(const std::string& filename)
{
	m_pImpl->m_filename = filename;
	try {
		//m_pImpl->m_node = YAML::LoadFile(filename);
		m_pImpl->m_isLoad = true;
	}
	catch (const std::exception& e) {
		qWarning() << e.what();
		return false;
	}
	return false;
}

bool CYamlParser::save()
{
	if (!m_pImpl->m_isLoad) return false;

	if(m_pImpl->m_filename.empty())
		throw CYamlParserException("filename is empty");
	try {
		std::ofstream out(m_pImpl->m_filename);
		//out << m_pImpl->m_node;
	}
	catch (const std::exception& e) {
		qWarning() << e.what();
		return false;
	}

	return true;
}

bool CYamlParser::set(const std::string& key, const std::string& value, const std::string& section)
{
	if (!m_pImpl->m_isLoad) return false;

	if (section.empty()) {
		//m_pImpl->m_node[key] = value;
		return true;
	}
	else {
		//m_pImpl->m_node[section][key] = value;
		return true;
	}
}

//template<typename T>
//std::string CYamlParser::get(const std::string& key, const std::string& section, const T& defaultValue)
//{
//	try {
//		if (section.empty()) {
//			return m_pImpl->m_node[key].as<std::string>();
//		}else {
//			return m_pImpl->m_node[section][key].as<std::string>();
//		}
//	}
//	catch (const std::exception& e) {
//		qWarning() << e.what();
//		return std::string();
//	}
//	return std::string();
//}

template<typename T>
inline T CYamlParser::get(const std::string& key, const std::string& section, T defaultValue)
{
	try {
		if (section.empty()) {
			//return m_pImpl->m_node[key].as<T>();
		}
		else {
			//return m_pImpl->m_node[section][key].as<T>();
		}
	}
	catch (const std::exception& e) {
		qWarning() << e.what();
		return T();
	}
	return bool();
}


//std::string CYamlParser::get(const std::string& key, 
//							const std::string& section,
//							const std::string& defaultValue)
//{
//	try {
//		if (section.empty()) {
//			return m_pImpl->m_node[key].as<std::string>();
//		}else {
//			return m_pImpl->m_node[section][key].as<std::string>();
//		}
//	}
//	catch (const std::exception& e) {
//		qWarning() << e.what();
//		return std::string();
//	}
//	return std::string();
//}


