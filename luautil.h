#pragma once

#define PHNT_VERSION PHNT_THRESHOLD // Windows 10

#include <phnt_windows.h>
#include <phnt.h>

typedef int( __stdcall *CCMLuaUtil_Release_t )( struct CCMLuaUtil * );
typedef int( __stdcall *CCMLuaUtil_LaunchInfSection_t )( struct CCMLuaUtil *, const wchar_t*, const wchar_t*, const wchar_t*, int );