#ifndef CONFILE_H
#define CONFILE_H
#include "CYamlParser.h"
#include <QString>

extern std::unique_ptr<CYamlParser> g_config;
extern QString g_exec_file_path;
extern QString g_exec_dir_path;

extern QString g_conf_path;
extern QString g_log_path;
#endif // CONFILE_H
