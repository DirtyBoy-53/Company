#pragma once
#include <string>
#include <memory>
#include <exception>

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
	T get(const std::string& key, const std::string& section = "", T defaultValue = T());
	//std::string get(const std::string& key, 
	//				const std::string& section = "", 
	//				const std::string& defaultValue = "");
	bool set(const std::string& key, const std::string& value, const std::string& section = "");


private:
	struct Impl;
	std::unique_ptr<Impl> m_pImpl;

};

