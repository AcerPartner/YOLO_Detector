#include "framework.h"
#include "Main.h"
#include "SocketServer.h"
#include "SocketClient.h"
#include "WinsockHelper.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_YOLODETECTOR, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_YOLODETECTOR));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;
    wcex.cbSize         = sizeof(WNDCLASSEX);
    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = nullptr;
    wcex.hIcon          = nullptr;
    wcex.hCursor        = nullptr;
    wcex.hbrBackground  = nullptr;
    wcex.lpszMenuName   = nullptr;
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = nullptr;

    return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowEx(WS_EX_NOACTIVATE, szWindowClass, szTitle, WS_POPUP, 0, 0, 0, 0, HWND_MESSAGE, NULL, NULL, NULL);
   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

// Test code, no error handling
std::vector<uint8_t> LoadModel(const std::string& modelPath)
{
    std::ifstream file(modelPath, std::ios::binary | std::ios::ate);
    if (!file)
    {
        OutputDebugStringA("[Main::LoadModel] Failed to open model file\n");
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<uint8_t> buffer(size);
    if (!file.read(reinterpret_cast<char*>(buffer.data()), size))
    {
        OutputDebugStringA("[Main::LoadModel] Failed to read model file\n");
    }
    return buffer;
}

void StartClient()
{
    std::thread t2([]()
    {
        cv::Mat image = cv::imread("lion.jpg"); // https://bing.gifposter.com/wallpaper-3074-lioncubs.html , 2024-Nov-21 Bing Wallpaper
        RunClient(image, LoadModel("yolov7-nms-640.onnx")); // https://github.com/WongKinYiu/yolov7/releases/download/v0.1/yolov7-nms-640.onnx
    });
    t2.detach();
}

void SocketTest()
{
    Sleep(500);
    // First test
    StartClient();
    Sleep(2000);
    // Second test to make sure the server keeps listening, and test concurrent connections
    StartClient();
    StartClient();
    StartClient();
    StartClient();
    StartClient();
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_CREATE:
		{
			OutputDebugStringA("[Main::WndProc] WM_CREATE\n");
            initializeWinsock();
            thread t(RunServer);
            t.detach();
            //SocketTest();
            break;
		}
		case WM_DESTROY:
        {
            PostQuitMessage(0);
            break;
        }
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}
