#pragma once
#include <aws/core/Region.h>
#include <stdio.h>
#include <stdint.h>
#include <memory>
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

static const char* AWS_ALLOC_TAG = "0";
template <typename T>
using AwsSharedPtr = std::shared_ptr<T>;

static const char * AWS_SERVER_REGION_1 = Aws::Region::US_WEST_1;

static const char * AWS_SERVER_REGION_2 = Aws::Region::US_WEST_2;