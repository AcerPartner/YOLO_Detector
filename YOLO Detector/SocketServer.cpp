#include "SocketServer.h"

void receiveMatAndModel(SOCKET clientSocket)
{
    int imageSize = 0, modelSize = 0;

    if (recv(clientSocket, (char*)&imageSize, sizeof(imageSize), 0) <= 0 ||
        recv(clientSocket, (char*)&modelSize, sizeof(modelSize), 0) <= 0)
    {
        OutputDebugStringA("[SocketServer] Failed to receive sizes.\n");
        return;
    }

    OutputDebugStringA("[SocketServer] Receiving image data...\n");

    vector<uint8_t> imageBuffer(imageSize);
    int bytesReceived = 0;

    while (bytesReceived < imageSize)
    {
        int chunkSize = recv(clientSocket, (char*)imageBuffer.data() + bytesReceived, imageSize - bytesReceived, 0);
        if (chunkSize <= 0)
        {
            OutputDebugStringA("[SocketServer] Failed to receive image data.\n");
            return;
        }
        bytesReceived += chunkSize;
    }

    OutputDebugStringA("[SocketServer] Image data received successfully!\n");

    cv::Mat image = cv::imdecode(imageBuffer, cv::IMREAD_COLOR);
    if (image.empty())
    {
        OutputDebugStringA("[SocketServer] Decoding image failed!\n");
        return;
    }

    OutputDebugStringA("[SocketServer] Receiving model data...\n");

    vector<uint8_t> modelData(modelSize);
    bytesReceived = 0;

    while (bytesReceived < modelSize)
    {
        int chunkSize = recv(clientSocket, (char*)modelData.data() + bytesReceived, modelSize - bytesReceived, 0);
        if (chunkSize <= 0)
        {
            OutputDebugStringA("[SocketServer] Failed to receive model data.\n");
            return;
        }
        bytesReceived += chunkSize;
    }

    vector<YOLO_output> detections;
    Inference in;
    in.detect(image, modelData, &detections);
    int numDetections = detections.size();
    send(clientSocket, (const char*)&numDetections, sizeof(numDetections), 0);
    send(clientSocket, (const char*)detections.data(), numDetections * sizeof(YOLO_output), 0);
}

void RunServer()
{
    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET)
    {
        OutputDebugStringA("[SocketServer] Could not create socket\n");
        return;
    }

    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    inet_pton(AF_INET, LOCALHOST, &serverAddr.sin_addr);
    serverAddr.sin_port = htons(SERVER_PORT);

    if (::bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
    {
        OutputDebugStringA("[SocketServer] Bind failed\n");
        closesocket(serverSocket);
        cleanupWinsock();
        return;
    }

    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR)
    {
        OutputDebugStringA("[SocketServer] Listen failed\n");
        closesocket(serverSocket);
        cleanupWinsock();
        return;
    }

    OutputDebugStringA("[SocketServer] Listening for incoming connections...\n");

    while (true)
    {
        struct sockaddr_in clientAddr;
        int clientAddrSize = sizeof(clientAddr);

        OutputDebugStringA("[SocketServer] Waiting for a client to connect...\n");
        SOCKET clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrSize);

		if (clientSocket == INVALID_SOCKET)
		{
			int error = WSAGetLastError();
			char errorMsg[256];
			sprintf_s(errorMsg, "[SocketServer] Accept failed. Error code: %d\n", error);
			OutputDebugStringA(errorMsg);

			if (error == WSAEINTR || error == WSAECONNRESET)
			{
				continue;
			}
			else
			{
				break;
			}
		}

        OutputDebugStringA("[SocketServer] Client connected, starting new thread.\n");

        thread clientThread(receiveMatAndModel, clientSocket);
        clientThread.detach();
    }

    closesocket(serverSocket);
    cleanupWinsock();
}