// Artifact.cpp : Defines the entry point for the application.
//

#include "pch.h"
#include "framework.h"
#include "Artifact.h"
#include "System/Engine.h"
#include "Rendering/IMGUIIncludes.h"

#define MAX_LOADSTRING 100

// Global Variables:
static Engine* g_Engine = nullptr;
HWND g_WindowHandle;
HINSTANCE g_Instance;                               // current instance
CHAR g_TitleStr[MAX_LOADSTRING];                    // The title bar text
CHAR g_WindowClassStr[MAX_LOADSTRING];              // the main window class name

// Forward declarations of functions included in this code module:
ATOM                RegisterWindowClass(HINSTANCE hInstance);
BOOL                InitialiseWindowInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    AboutWndProc(HWND, UINT, WPARAM, LPARAM);

int APIENTRY WinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

#if defined(DEBUG) | defined(_DEBUG)
    if (!AttachConsole(ATTACH_PARENT_PROCESS))
        AllocConsole();

    FILE* file = nullptr;
    freopen_s(&file, "CONIN$", "r", stdin);
    freopen_s(&file, "CONOUT$", "w", stdout);
    freopen_s(&file, "CONOUT$", "w", stderr);

    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

    // Initialize global strings
    LoadString(hInstance, IDS_APP_TITLE, g_TitleStr, MAX_LOADSTRING);
    LoadString(hInstance, IDC_ARTIFACT, g_WindowClassStr, MAX_LOADSTRING);
    RegisterWindowClass(hInstance);

    // Perform application initialization:
    if (!InitialiseWindowInstance (hInstance, nCmdShow))
    {
        return -1;
    }

    std::string contentFolderPath = "Content\\";
    g_Engine = Engine::CreateEngine(g_WindowHandle, contentFolderPath);

    if (g_Engine == nullptr)
    {
        printf("Failed to create engine. Exitting WinMain.\n");
        return -1;
    }

    g_Engine->Start();

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_ARTIFACT));

    MSG msg{};

    auto lTime = std::chrono::steady_clock::now();
    auto cTime = lTime;
    std::chrono::duration<long double> clockDelta = { };
    long double deltaTime = 0.0;
    //float accumulator = 0.0f;
    //int fixedUpdates = 0;
    //int variableUpdates = 0;

    while (g_Engine->IsRunning())
    {
        while (PeekMessage(&msg, g_WindowHandle, 0, 0, PM_REMOVE))
        {
            if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }

        cTime = std::chrono::steady_clock::now();
        clockDelta = std::chrono::duration_cast<std::chrono::milliseconds>(cTime - lTime);
        deltaTime = clockDelta.count();

        //if (deltaTime > CAPPED_TIMESTEP)
        //{
        //    deltaTime = CAPPED_TIMESTEP;
        //}

        //accumulator += deltaTime;

        //while (accumulator >= FIXED_TIMESTEP)
        //{
        //    fixedUpdates++;
        //    accumulator -= deltaTime;
        //}

        g_Engine->FixedUpdate();
        g_Engine->Update(deltaTime);
        //variableUpdates++;

        g_Engine->Render();

        lTime = cTime;

        g_Engine->CalculateTimings();

        //fixedUpdates = 0;
        //variableUpdates = 0;
    }
 
    if (Engine::DestroyEngine(g_Engine) == true)
    {
        printf("Destroyed engine successfully.\n");
    }
    
    g_Engine = nullptr;

    return (int) msg.wParam;
}

// Registers class window
ATOM RegisterWindowClass(HINSTANCE instance)
{
    WNDCLASSEX wcex;
    memset(&wcex, 0, sizeof(WNDCLASSEX));
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = instance;
    wcex.hIcon          = LoadIcon(instance, MAKEINTRESOURCE(IDI_ARTIFACT));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCE(IDC_ARTIFACT);
    wcex.lpszClassName  = g_WindowClassStr;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassEx(&wcex);
}

//Initialises Window Instance and stores handle.
BOOL InitialiseWindowInstance(HINSTANCE instance, int nCmdShow)
{
   g_Instance = instance; // Store instance handle in our global variable

   HWND hWnd = CreateWindow(g_WindowClassStr, g_TitleStr, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, instance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   g_WindowHandle = hWnd;

   return TRUE;
}

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

//Generic WndProc
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam))
        return true;

    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(g_Instance, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, AboutWndProc);
                break;
            case IDM_EXIT:
                g_Engine->Stop();
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;

    case WM_DESTROY:
        g_Engine->Stop();
        PostQuitMessage(0);
        break;

    default:
        if (g_Engine != nullptr)
        {
            return g_Engine->WndProc(hWnd, message, wParam, lParam);
        }
        else
        {
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
        break;
    }

    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK AboutWndProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
