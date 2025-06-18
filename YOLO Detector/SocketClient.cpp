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
    int numDetections = 0;
    if (recv(socket, (char*)&numDetections, sizeof(numDetections), 0) <= 0)
    {
        OutputDebugStringA("[SocketClient] Failed to receive number of detections.\n");
        return;
    }
    OutputDebugStringA(("[SocketClient] numDetections: " + to_string(numDetections) + "\n").c_str());

    vector<YOLO_output> server_output(numDetections);
    int bytesReceived = 0;
    while (bytesReceived < numDetections * sizeof(YOLO_output))
    {
        int chunkSize = recv(socket, (char*)server_output.data() + bytesReceived, (numDetections * sizeof(YOLO_output)) - bytesReceived, 0);
        if (chunkSize <= 0)
        {
            OutputDebugStringA("[SocketClient] Failed to receive detections.\n");
            return;
        }
        bytesReceived += chunkSize;
    }

    for (const YOLO_output& out : server_output)
    {
        char responseMsg[256];
        sprintf_s(responseMsg, "[SocketClient] Detected: batch_id=%.1f, x0=%.1f, y0=%.1f, x1=%.1f, y1=%.1f, cls_id=%.1f, score=%.2f, msg=%s\n",
                  out.batch_id, out.x0, out.y0, out.x1, out.y1, out.cls_id, out.score, out.msg.c_str());

        OutputDebugStringA(responseMsg);
    }
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