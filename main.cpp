#define _WIN32_WINNT 0x0500 //Needed for GetConsoleWindow()
#include <iostream>
#include <windows.h>
#include <gl/gl.h>
#include <fcntl.h> //for console
#include <stdio.h>
#include <ctime>

#include "definitions.h"
#include "game.h"
#include "resource.h"

/*Include following libraries
lobSOIL.a
OpenAL32.lib
XInput32.lib
XInput64.lib
opengl32
glu32
gdi32
ogg.lib
vorbis.lib
vorbisfile.lib
*/

int*  g_pWindow_size;
bool* g_pKeys_real;
bool* g_pKeys_translated;
int*  g_pMouse_pos;
bool* g_pMouse_but;

using namespace std;

LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);
void EnableOpenGL(HWND hwnd, HDC*, HGLRC*);
void DisableOpenGL(HWND, HDC, HGLRC);


int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine,
                   int nCmdShow)
{
    string command_line=lpCmdLine;
    bool debug_mode=false;
    if(command_line=="debug") debug_mode=true;

    //init and reset keys
    g_pWindow_size=new int[2];
    g_pWindow_size[0]=1024;//1024
    g_pWindow_size[1]=1080;//576
    g_pKeys_real=new bool[256];
    g_pKeys_translated=new bool[256];
    for(int i=0;i<256;i++)
    {
        g_pKeys_real[i]=false;
        g_pKeys_translated[i]=false;
    }
    g_pMouse_pos=new int[2];
    g_pMouse_pos[0]=0; g_pMouse_pos[1]=0;
    g_pMouse_but=new bool[4];
    g_pMouse_but[0]=g_pMouse_but[1]=g_pMouse_but[2]=g_pMouse_but[3]=false;

    WNDCLASSEX wcex;
    HWND hwnd;
    HDC hDC;
    HGLRC hRC;
    MSG msg;

    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_OWNDC;
    wcex.lpfnWndProc = WindowProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance,MAKEINTRESOURCE(IDI_LUD));
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = "LD33";
    wcex.hIconSm = LoadIcon(NULL, IDI_APPLICATION);;


    if (!RegisterClassEx(&wcex))
        return 0;

    if(!debug_mode)
    {
        //Detect screen resolution
        RECT desktop;
        // Get a handle to the desktop window
        const HWND hDesktop = GetDesktopWindow();
        // Get the size of screen to the variable desktop
        GetWindowRect(hDesktop, &desktop);
        // The top left corner will have coordinates (0,0)
        // and the bottom right corner will have coordinates
        // (horizontal, vertical)
        g_pWindow_size[0] = desktop.right;
        g_pWindow_size[1] = desktop.bottom;
    }

    //if debug mode start console
    if(debug_mode)
    {
        //Open a console window
        AllocConsole();
        //Connect console output
        HANDLE handle_out = GetStdHandle(STD_OUTPUT_HANDLE);
        int hCrt          = _open_osfhandle((long) handle_out, _O_TEXT);
        FILE* hf_out      = _fdopen(hCrt, "w");
        setvbuf(hf_out, NULL, _IONBF, 1);
        *stdout = *hf_out;
        //Connect console input
        HANDLE handle_in = GetStdHandle(STD_INPUT_HANDLE);
        hCrt             = _open_osfhandle((long) handle_in, _O_TEXT);
        FILE* hf_in      = _fdopen(hCrt, "r");
        setvbuf(hf_in, NULL, _IONBF, 128);
        *stdin = *hf_in;
        //Set console title
        SetConsoleTitle("Debug Console");
        HWND hwnd_console=GetConsoleWindow();
        MoveWindow(hwnd_console,g_pWindow_size[0],0,680,510,TRUE);

        cout<<"Software started\n";
        cout<<"Version: "<<_version<<endl;
    }
    else
    {
        ShowCursor(FALSE);//hide cursor
        //ShowWindow(GetConsoleWindow(),SW_HIDE);//hide console
    }

    hwnd = CreateWindowEx(0,
                          "LD33",
                          "LD33",
                          WS_VISIBLE | WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, //WS_OVERLAPPEDWINDOW for window
                          CW_USEDEFAULT,
                          CW_USEDEFAULT,
                          g_pWindow_size[0],
                          g_pWindow_size[1],
                          NULL,
                          NULL,
                          hInstance,
                          NULL);

    ShowWindow(hwnd, nCmdShow);

    EnableOpenGL(hwnd, &hDC, &hRC);

    //start the game
    game Game;
    if( !Game.init(&g_pWindow_size[0],&g_pKeys_real[0],&g_pKeys_translated[0],
                   &g_pMouse_pos[0],&g_pMouse_but[0]) )
    {
        cout<<"ERROR: Could not initaialize the game\n";
        if(debug_mode) system("PAUSE");
        return 1;
    }
    clock_t time_now=clock();
    clock_t time_last=time_now;
    //clock_t time_sum=0;
    clock_t time_step=_time_step*1000.0;//10.0;//0.010 ms -> 100 updates per sec
    bool update_screen=true;

    bool quit=false;
    while(!quit)
    {
        if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if(msg.message==WM_QUIT)
            {
                quit=true;
            }
            else
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        else
        {
            //quick quit test
            //if(g_pKeys_real[VK_ESCAPE]) quit=true;

            time_now=clock();
            while( time_last+time_step <= time_now )
            {
                time_last+=time_step;

                bool quit_game=false;
                if( !Game.update(quit_game) )//static update
                {
                    //require reset of game
                    if( !Game.init(&g_pWindow_size[0],&g_pKeys_real[0],&g_pKeys_translated[0],
                                   &g_pMouse_pos[0],&g_pMouse_but[0],true) )
                    {
                        cout<<"ERROR: Game could not reinitialize\n";
                        if(debug_mode) system("PAUSE");
                        return 1;
                    }
                }
                if(quit_game) quit=true;

                update_screen=true;
            }
            //draw, if anything updated
            if(update_screen)
            {
                update_screen=false;

                Game.draw();
                SwapBuffers(hDC);
            }
        }
    }

    //clean up
    delete[] g_pWindow_size;
    delete[] g_pKeys_real;
    delete[] g_pKeys_translated;
    delete[] g_pMouse_pos;
    delete[] g_pMouse_but;

    DisableOpenGL(hwnd, hDC, hRC);

    DestroyWindow(hwnd);

    return msg.wParam;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_CLOSE:
        {
            PostQuitMessage(0);
        }
        break;

        case WM_DESTROY:
        {
            return 0;
        }
        break;

        case WM_MOUSEMOVE:
        {
             g_pMouse_pos[0]=LOWORD(lParam);
             g_pMouse_pos[1]=HIWORD(lParam)+28;//-22 pixel shift to get same coord as drawing
        }
        break;

        case WM_LBUTTONDOWN:
        {
             g_pMouse_but[0]=true;
        }
        break;

        case WM_LBUTTONUP:
        {
             g_pMouse_but[0]=false;
        }
        break;

        case WM_RBUTTONDOWN:
        {
             g_pMouse_but[1]=true;

             //cout<<"x: "<<g_pMouse_pos[0]<<", y: "<<g_pMouse_pos[1]<<endl; //temp
        }
        break;

        case WM_RBUTTONUP:
        {
             g_pMouse_but[1]=false;
        }
        break;

        case WM_MOUSEWHEEL:
        {
            if(HIWORD(wParam)>5000) {g_pMouse_but[2]=true;}
            if(HIWORD(wParam)>100&&HIWORD(wParam)<5000) {g_pMouse_but[3]=true;}
        }
        break;

		case WM_KEYDOWN:
		{
			g_pKeys_real[wParam]=true;

			//cout<<"Pressed: "<<wParam<<endl;
		}
		break;

		case WM_KEYUP:
		{
			g_pKeys_real[wParam]=false;
		}
		break;

        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    return 0;
}

void EnableOpenGL(HWND hwnd, HDC* hDC, HGLRC* hRC)
{
    PIXELFORMATDESCRIPTOR pfd;

    int iFormat;

    /* get the device context (DC) */
    *hDC = GetDC(hwnd);

    /* set the pixel format for the DC */
    ZeroMemory(&pfd, sizeof(pfd));

    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW |
                  PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 24;
    pfd.cDepthBits = 16;
    pfd.cStencilBits = 8;
    pfd.iLayerType = PFD_MAIN_PLANE;

    iFormat = ChoosePixelFormat(*hDC, &pfd);

    SetPixelFormat(*hDC, iFormat, &pfd);

    /* create and enable the render context (RC) */
    *hRC = wglCreateContext(*hDC);

    wglMakeCurrent(*hDC, *hRC);

    //set 2D mode

    glClearColor(0.0,0.0,0.0,0.0);  //Set the cleared screen colour to black
    glViewport(0,0,g_pWindow_size[0],g_pWindow_size[1]);   //This sets up the viewport so that the coordinates (0, 0) are at the top left of the window

    //Set up the orthographic projection so that coordinates (0, 0) are in the top left
    //and the minimum and maximum depth is -10 and 10. To enable depth just put in
    //glEnable(GL_DEPTH_TEST)
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0,g_pWindow_size[0],g_pWindow_size[1],0,-1,1);

    //Back to the modelview so we can draw stuff
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    //Enable antialiasing
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_POLYGON_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT,GL_NICEST);
    glHint(GL_POLYGON_SMOOTH_HINT,GL_NICEST);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClearStencil( 0 );
}

void DisableOpenGL (HWND hwnd, HDC hDC, HGLRC hRC)
{
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(hRC);
    ReleaseDC(hwnd, hDC);
}

