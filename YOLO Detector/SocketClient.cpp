#include "SocketClient.h"

void sendMatAndModel(SOCKET socket, const cv::Mat& mat, const vector<uint8_t>& modelData)
{
    if (mat.empty())
    {
        OutputDebugStringA("[SocketClient] Empty image, not sending.\n");
        return;
    }
    vector<uint8_t> imageBuffer;
    cv::imencode(".jpg", mat, imageBuffer);
    int imageSize = imageBuffer.size();
    int modelSize = modelData.size();

    // Send sizes first
    if (send(socket, (const char*)&imageSize, sizeof(imageSize), 0) == SOCKET_ERROR ||
        send(socket, (const char*)&modelSize, sizeof(modelSize), 0) == SOCKET_ERROR)
    {
        OutputDebugStringA("[SocketClient] Failed to send sizes.\n");
        return;
    }

    // Send image data
    int totalBytesSent = 0;
    while (totalBytesSent < imageSize)
    {
        int bytesSent = send(socket, (const char*)imageBuffer.data() + totalBytesSent, imageSize - totalBytesSent, 0);
        if (bytesSent == SOCKET_ERROR)
        {
            OutputDebugStringA("[SocketClient] Failed to send image data.\n");
            return;
        }
        totalBytesSent += bytesSent;
    }

    // Send model binary data
    totalBytesSent = 0;
    while (totalBytesSent < modelSize)
    {
        int bytesSent = send(socket, (const char*)modelData.data() + totalBytesSent, modelSize - totalBytesSent, 0);
        if (bytesSent == SOCKET_ERROR)
        {
            OutputDebugStringA("[SocketClient] Failed to send model data.\n");
            return;
        }
        totalBytesSent += bytesSent;
    }

    OutputDebugStringA("[SocketClient] Image and model data sent successfully!\n");
}

void receiveResponse(SOCKET socket)
{
    size_t count = 0;
    if (recv(socket, (char*)&count, sizeof(count), 0) <= 0)
    {
        OutputDebugStringA("[SocketClient] Failed to receive count.\n");
        return;
    }
    OutputDebugStringA(("[SocketClient] count: " + to_string(count) + "\n").c_str());

    float* output = new float[count]();
    int bytesReceived = 0;
    while (bytesReceived < count * sizeof(float))
    {
        int chunkSize = recv(socket, (char*)output + bytesReceived, (count * sizeof(float)) - bytesReceived, 0);
        if (chunkSize <= 0)
        {
            OutputDebugStringA("[SocketClient] Failed to receive output.\n");
            return;
        }
        bytesReceived += chunkSize;
    }

    // Do some output data process here

    delete[] output;
    char responseMsg[256];
    sprintf_s(responseMsg, "[SocketClient] Received bytes: %d, count: %zu\n", bytesReceived, count);
    OutputDebugStringA(responseMsg);
}

void RunClient(const cv::Mat& image, const vector<uint8_t>& modelData)
{
    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET)
    {
        OutputDebugStringA("[SocketClient] Could not create socket\n");
        cleanupWinsock();
        return;
    }

    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    inet_pton(AF_INET, LOCALHOST, &serverAddr.sin_addr);
    serverAddr.sin_port = htons(SERVER_PORT);

    if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0)
    {
        OutputDebugStringA("[SocketClient] Connect error\n");
        closesocket(clientSocket);
        cleanupWinsock();
        return;
    }

    OutputDebugStringA("[SocketClient] Connected to server\n");

    sendMatAndModel(clientSocket, image, modelData);
    receiveResponse(clientSocket);

    closesocket(clientSocket);
    //cleanupWinsock(); // Uncomment when this line when SocketClient is NOT in the same process as SocketServer.
    OutputDebugStringA("[SocketClient] Finished\n");
}