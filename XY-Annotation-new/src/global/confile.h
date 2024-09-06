#ifndef HPLAYER_CONFILE_H_
#define HPLAYER_CONFILE_H_

#include <qglobal.h>
//#pragma execution_character_set("utf-8")

#include "iniparser.h"

extern IniParser* g_config;
extern char g_exec_path[256];
extern char g_exec_dir[256];
extern char g_run_dir[256];

extern char g_conf_file[256];
extern char g_log_file[256];

#endif  // HPLAYER_CONFILE_H_

