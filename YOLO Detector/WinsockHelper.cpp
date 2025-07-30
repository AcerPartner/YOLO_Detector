#include "WinsockHelper.h"

#pragma comment(lib, "ws2_32.lib")

void initializeWinsock()
{
    if (!initialized)
    {
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
        {
            OutputDebugStringA("[WinsockHelper::initializeWinsock] WSAStartup failed\n");
            exit(1);
        }
        initialized = true;
    }
}

void cleanupWinsock()
{
    WSACleanup();
    initialized = false;
}