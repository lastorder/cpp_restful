#pragma once

#include "../inc/log_base.h"

extern LogFuc g_logfuc;

#define LOG_TRACE_D(msg) LOG_FORMAT_MSG(g_logfuc,0,"HTTP_MODULE",msg)
#define LOG_TRACE_E(msg) LOG_FORMAT_MSG(g_logfuc,4,"HTTP_MODULE",msg)
