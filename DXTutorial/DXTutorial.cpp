// DXTutorial.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "DXTutorial.h"
#include "Math/Vector4.h"
#include "FrontEndRenderer.h"
#include "Model/Model.h"
#include "Time.h"
#include "KeyboardInput.h"
#include "imgui.h"

using namespace jcl;

Vertex triangle[3] = {
  { {  1.0f,  0.0f, 0.0f, 1.0f }, { 1.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } },
  { { -1.0f,  0.0f, 0.0f, 1.0f }, { 1.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } },
  { {  1.0f,  1.0f, 0.0f, 1.0f }, { 1.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } },
};

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
HWND                windowHandle;
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
BOOL                bShouldClose;
jcl::FrontEndRenderer* pRenderer;


void pollEvent()
{
    
  MSG msg;
  // Main message loop:
  while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
  }
}


void initializeEngine(HWND window)
{
  pRenderer = new jcl::FrontEndRenderer();
  pRenderer->init(window, jcl::FrontEndRenderer::RENDERER_RHI_D3D_12);
  Time::initialize();
}


void cleanUpEngine()
{
  pRenderer->cleanUp();
  delete pRenderer;
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_DXTUTORIAL, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);
    bShouldClose = false;

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    initializeEngine(windowHandle);

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_DXTUTORIAL));

    jcl::Model model;
    jcl::Model model1;
    jcl::Model model2;
    model.initialize("SongWork/spartan.obj", pRenderer);
    model1.initialize("SongWork/OldCar.obj", pRenderer);
    model2.initialize("SongWork/RacingCar.obj", pRenderer);

    jcl::Globals globals;
    pRenderer->setGlobals(&globals);
    globals._targetSize[0] = 1920;
    globals._targetSize[1] = 1080;
    globals._targetSize[2] = 0;
    globals._targetSize[3] = 0;

    VertexBuffer buffer = pRenderer->createVertexBuffer(triangle, sizeof(Vertex), sizeof(triangle));
    RenderUUID transformId = pRenderer->createTransformBuffer();
    RenderUUID transformId1 = pRenderer->createTransformBuffer();
    RenderUUID transformId2 = pRenderer->createTransformBuffer();
    RenderUUID materialId = pRenderer->createMaterialBuffer();
    PerMeshDescriptor descriptor = { };
    PerMeshDescriptor descriptor1 = { };
    PerMeshDescriptor descriptor2 = { };

    PerMaterialDescriptor mat = { };
    mat._color = Vector4(1.0f, 0.0f, 0.0f);
    mat._matrialFlags = 0;
    mat._albedoFactor = Vector4(1.0f, 1.0f, 1.0f);

    // Use same descriptor for both.
    GeometryMesh mesh = { };
    mesh._vertexBufferView = model.getVertexBufferView();
    mesh._indexBufferView = model.getIndexBufferView();
    mesh._meshTransform = &descriptor;
    mesh._meshDescriptor = transformId;
    mesh._submeshCount = 1;

    GeometryMesh mesh1 = { };
    mesh1._vertexBufferView = model1.getVertexBufferView();
    mesh1._indexBufferView = model1.getIndexBufferView();
    mesh1._meshTransform = &descriptor1;
    mesh1._meshDescriptor = transformId1;
    mesh1._submeshCount = 1;

    GeometryMesh mesh2 = { };
    mesh2._vertexBufferView = model2.getVertexBufferView();
    mesh2._indexBufferView = model2.getIndexBufferView();
    mesh2._meshTransform = &descriptor2;
    mesh2._meshDescriptor = transformId2;
    mesh2._submeshCount = 1;

    GeometrySubMesh submesh = { };
    submesh._materialDescriptor = materialId;
    submesh._matData = &mat;
    submesh._startVert = model.getSubMesh(0)->m_vertOffset;
    submesh._vertCount = model.getSubMesh(0)->m_vertCount;
    submesh._indCount = model.getSubMesh(0)->m_indCount;
    submesh._indOffset = model.getSubMesh(0)->m_indOffset;
    submesh._vertInst = 1;

    GeometrySubMesh submesh1 = { };
    submesh1._materialDescriptor = materialId;
    submesh1._matData = &mat;
    submesh1._startVert = model1.getSubMesh(0)->m_vertOffset;
    submesh1._vertCount = model1.getSubMesh(0)->m_vertCount;
    submesh1._indCount = model1.getSubMesh(0)->m_indCount;
    submesh1._indOffset = model1.getSubMesh(0)->m_indOffset;
    submesh1._vertInst = 1;

    GeometrySubMesh submesh2 = { };
    submesh2._materialDescriptor = materialId;
    submesh2._matData = &mat;
    submesh2._startVert = model2.getSubMesh(0)->m_vertOffset;
    submesh2._vertCount = model2.getSubMesh(0)->m_vertCount;
    submesh2._indCount = model2.getSubMesh(0)->m_indCount;
    submesh2._indOffset = model2.getSubMesh(0)->m_indOffset;
    submesh2._vertInst = 1;

R32 g = 0.0f;
    while (!bShouldClose) {
        Time::update();
        pollEvent();
        Time time;
        R32 t = time.getTimeStamp();
        
        globals._cameraPos = { 0.0f, /*sinf(t * 0.0000001f) */ 10.0f, 55.0f, 1.0f };
        Matrix44 P = m::Matrix44::perspectiveRH(ToRads(60.0f), 1920.0f / 1080.0f, 0.01f, 1000.0f);
        
        Matrix44 r = Matrix44::rotate(Matrix44(), ToRads(220.0f), Vector3(0.0f, 1.0f, 0.0f));
        Matrix44 V = Matrix44::translate(Matrix44(), Vector4(-g * 0.01f, 5.0f, 50.0f, 1.0f)) * r;
        globals._viewToClip = V * P;
        g += 1.0f;
        R32 ss = (sinf(t * 0.0000001f) * 0.5f + 0.5f) * 2.0f;
        Matrix44 R = Matrix44(); //Matrix44::rotate(Matrix44(), ToRads(t * 0.000001f), Vector4(0.0f, 1.0f, 0.0f));
        Matrix44 T = Matrix44();//Matrix44::translate(Matrix44::rotate(Matrix44(), ToRads(90.0f), Vector3(1.0f, 0.0f, 0.0f)), Vector4(0.0f, 0.0f, 0.0f));
        Matrix44 S = Matrix44();
        Matrix44 W = S * R * T;
        Matrix44 W1 = Matrix44::translate(Matrix44(), Vector4(-20.f, 0.0f, 0.0f));
        Matrix44 W2 = Matrix44::translate(Matrix44(), Vector4(20.f, 0.0f, 0.0f));
                             
        descriptor._previousWorldToViewClip = descriptor._worldToViewClip;
        descriptor._worldToViewClip = W * globals._viewToClip;
        descriptor._world = W;
        descriptor._n = descriptor._world;
        descriptor._n[3][0] = 0.0f;
        descriptor._n[3][1] = 0.0f;
        descriptor._n[3][2] = 0.0f;
        descriptor._n = descriptor._n.inverse().transpose();

        descriptor1._previousWorldToViewClip = descriptor1._worldToViewClip;
        descriptor1._worldToViewClip = W1 * globals._viewToClip;
        descriptor1._world = W1;
        descriptor1._n = descriptor1._world;
        descriptor1._n[3][0] = 0.0f;
        descriptor1._n[3][1] = 0.0f;
        descriptor1._n[3][2] = 0.0f;
        descriptor1._n = descriptor1._n.inverse().transpose();

        descriptor2._previousWorldToViewClip = descriptor2._worldToViewClip;
        descriptor2._worldToViewClip = W2 * globals._viewToClip;
        descriptor2._world = W2;
        descriptor2._n = descriptor2._world;
        descriptor2._n[3][0] = 0.0f;
        descriptor2._n[3][1] = 0.0f;
        descriptor2._n[3][2] = 0.0f;
        descriptor2._n = descriptor2._n.inverse().transpose();

        GeometrySubMesh* submeshes[] = { &submesh };
        GeometrySubMesh* submeshes1[] = { &submesh1 };
        GeometrySubMesh* submeshes2[] = { &submesh2 };
        pRenderer->pushMesh(&mesh, submeshes);
        pRenderer->pushMesh(&mesh1, submeshes1);
        pRenderer->pushMesh(&mesh2, submeshes2);
        pRenderer->update(0.0f, globals);
        pRenderer->render();
    }

    cleanUpEngine();
    return 0;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_DXTUTORIAL));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_DXTUTORIAL);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);
    windowHandle = hWnd;
   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        bShouldClose = true;
        break;
    case WM_KEYDOWN:
        {
            SHORT state = GetKeyState(wParam);
            UINT key = LOWORD(wParam);
            Keyboard::registerInput(key, INPUT_STATUS_DOWN);
        } 
        break;
    case WM_KEYUP:
        {
            SHORT state = GetKeyState(wParam);
            UINT key = LOWORD(wParam);
            Keyboard::registerInput(key, INPUT_STATUS_UP);
        }
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
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
