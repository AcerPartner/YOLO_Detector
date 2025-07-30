#pragma once
#include <opencv2/opencv.hpp>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <vector>
#include "WinsockHelper.h"
#include "Inference.h"

using namespace std;

#define LOCALHOST "127.0.0.1"
#define SERVER_PORT 50226

void receiveMatAndModel(SOCKET clientSocket);
void RunServer();