#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <shellapi.h>
#include <TlHelp32.h>
#include <comdef.h>

#include <iostream>
#include <filesystem>

using namespace std;

bool injectDLL(const int& pid);
int getProcID(const string& p_name);
string getCurrentDir();