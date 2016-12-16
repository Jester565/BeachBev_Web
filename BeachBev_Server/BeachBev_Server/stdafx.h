#pragma once
#include <stdio.h>
#ifdef _WIN32
#include <WinSock2.h>
#define OTL_ODBC_MSSQL_2008
#else
#define OTL_ODBC_UNIX
#endif
#define OTL_STL
#include "otlv4.h"
