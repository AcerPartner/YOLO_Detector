#pragma once
#include <opencv2/opencv.hpp>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <vector>
#include "WinsockHelper.h"
#include "YOLO.h"
#include "Inference.h"

#define LOCALHOST "127.0.0.1"
#define SERVER_PORT 50226

void initializeWinsock();
void receiveMatAndModel(SOCKET clientSocket);
void RunServer();