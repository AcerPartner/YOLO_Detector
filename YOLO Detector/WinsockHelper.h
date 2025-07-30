#pragma once
#include <winsock2.h>
#include <windows.h>

void initializeWinsock();
void cleanupWinsock();

static bool initialized = false;