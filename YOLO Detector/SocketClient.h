#pragma once
#include <opencv2/opencv.hpp>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <vector>
#include <fstream>
#include "WinsockHelper.h"
#include "YOLO.h"

#define LOCALHOST "127.0.0.1"
#define SERVER_PORT 50226

void initializeWinsock();
void sendMatAndModel(SOCKET socket, const cv::Mat& mat, const std::vector<uint8_t>& modelData);
void receiveResponse(SOCKET socket);
void RunClient(const cv::Mat& image, const std::vector<uint8_t>& modelData);