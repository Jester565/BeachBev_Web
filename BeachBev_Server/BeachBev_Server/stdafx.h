#pragma once
#include <stdio.h>
#include <stdint.h>
#ifdef _WIN32
#include <WinSock2.h>
#define OTL_BIGINT int64_t
#define OTL_ODBC_MSSQL_2008
#else
#define OTL_ODBC_UNIX
#define OTL_BIGINT long long
#endif
#define OTL_STL
#include "otlv4.h"
