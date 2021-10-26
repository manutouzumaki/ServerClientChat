#include "socket.h"
#include <stdio.h>

// We are going to try builing a online chat
// is going to be a client-server aproach
// this is the code for the graphical interface
// and the client conection, lets start...

#define IDC_hBU_Join 40001
#define IDC_hLB_Output 40002
#define IDC_hEB_Name 40003
#define IDC_hEB_IP 40004
#define IDC_hEB_Port 40005
#define IDC_hEB_Text 40006

HWND hBU_Join = NULL;
HWND hLB_Output = NULL;
HWND hEB_Name = NULL;
HWND hEB_IP = NULL;
HWND hEB_Port = NULL;
HWND hEB_Text = NULL;

struct ShearMemory
{
    int AppRunnign;
    SocketObject *mClientSocketObject;
    HWND *hLB_Output;
};

LPCTSTR lpszApplicationName = "ListBoxProg";
LPCTSTR lpszTitle = "ListBox Example";

static SocketObject ClientSocketObject;
static ShearMemory Memory;
static bool IsConected;

static LPDWORD RecvThreadID = NULL;
static HANDLE RecvThreadHandle;


// Message Loop CallBack Function prototype
LRESULT CALLBACK MessageProcessor(HWND Window, UINT Message, WPARAM WParam, LPARAM LParam);
void ConnectClient(char *User, char *IP, int Port);

DWORD WINAPI AsyncRecvFunc(LPVOID Param);

int WINAPI WinMain(HINSTANCE Instance, HINSTANCE PrevInstance, LPSTR CmdLine, int CmdShow)
{    
    MSG Message;
    HWND Window;
    WNDCLASSEX WndClass;

    // Set up window attributes
    WndClass.cbSize = sizeof(WndClass);
    WndClass.style = CS_HREDRAW | CS_VREDRAW;
    WndClass.lpfnWndProc = MessageProcessor;
    WndClass.cbClsExtra = 0;
    WndClass.cbWndExtra = 0;
    WndClass.hInstance = Instance;
    WndClass.hIcon = LoadIcon( NULL, IDI_APPLICATION );
    WndClass.hCursor = LoadCursor( NULL, IDC_ARROW );
    WndClass.hbrBackground = (HBRUSH)(COLOR_WINDOW);
    WndClass.lpszMenuName = NULL;
    WndClass.lpszClassName = lpszApplicationName;
    WndClass.hIconSm = LoadIcon( NULL, IDI_APPLICATION );
    if(RegisterClassEx(&WndClass) == 0)
    {
        exit(1);
    }
    
    // Create the window
    Window = CreateWindow(lpszApplicationName, lpszTitle, WS_OVERLAPPEDWINDOW,
                          100, 100, 400, 340, NULL, NULL, Instance, NULL);
    hBU_Join = CreateWindow("BUTTON", "Join", WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON,
                                  275, 270, 100, 28, Window, (HMENU)IDC_hBU_Join, Instance, NULL);
    hLB_Output = CreateWindowEx(WS_EX_CLIENTEDGE, "LISTBOX", NULL, 
                                WS_CHILD|WS_VISIBLE|LBS_NOTIFY|WS_VSCROLL|WS_BORDER,
                                2, 37, 380, 230, Window, (HMENU)IDC_hLB_Output, Instance, NULL);
    hEB_Name = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "Name...",
                              WS_CHILD|WS_VISIBLE|WS_BORDER|ES_LEFT,
                              250, 7, 135, 28, Window, (HMENU)IDC_hEB_Name, Instance, NULL);
    hEB_IP = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "IP...",
                            WS_CHILD|WS_VISIBLE|WS_BORDER|ES_LEFT,
                            2, 7, 125, 28, Window, (HMENU)IDC_hEB_IP, Instance, NULL);
    hEB_Port = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "Port...",
                              WS_CHILD|WS_VISIBLE|WS_BORDER|ES_LEFT,
                              130, 7, 75, 28, Window, (HMENU)IDC_hEB_Port, Instance, NULL);
    hEB_Text = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "Chat...",
                              WS_CHILD|WS_VISIBLE|WS_BORDER|ES_LEFT,
                              2, 270, 270, 28, Window, (HMENU)IDC_hEB_Text, Instance, NULL);

    ShowWindow(Window, CmdShow);
    UpdateWindow(Window);

    // Process Messages until the program is terminated
    while(GetMessage(&Message, NULL, 0, 0))
    {
        TranslateMessage(&Message);
        DispatchMessage(&Message);
    }
    
    Memory.AppRunnign = false;
    CloseHandle(RecvThreadHandle);

    return(Message.wParam); 
}

// Callback function to handle window messages
LRESULT CALLBACK MessageProcessor(HWND Window, UINT Message, WPARAM WParam, LPARAM LParam)
{
    char User[256];
    char IP[256];
    char PortCharBuffer[256];
    int Port = 0;

    switch(Message)
    {
        case WM_COMMAND:
            switch(LOWORD(WParam))
            {
                case IDC_hBU_Join:
                    if(!IsConected)
                    {
                        GetWindowText(hEB_IP, IP, 256);
                        GetWindowText(hEB_Port, PortCharBuffer, 256);
                        GetWindowText(hEB_Name, User, 256);
                        Port = atoi(PortCharBuffer);
                        PlaySound("../data/join.wav", NULL, SND_FILENAME);
                        ConnectClient(User, IP, Port);
                        if(IsConected)
                        {
                            Memory.AppRunnign = true;
                            Memory.hLB_Output = &hLB_Output;
                            Memory.mClientSocketObject = &ClientSocketObject;
                            RecvThreadHandle = CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)AsyncRecvFunc,
                                                            (void *)&Memory, CREATE_SUSPENDED, RecvThreadID);
                            ResumeThread(RecvThreadHandle);
                        }
                    }
                    else
                    {
                        // If We are connected Send the data to the server
                        GetWindowText(hEB_Name, User, 256);
                        GetWindowText(hEB_Text, IP, 256);
                        char Buffer[256];
                        int BytesSend = 0;
                        sprintf(Buffer, "User <%s>: %s", User, IP);
                        BytesSend = ClientSocketObject.Send(Buffer, 256, 0);
                        //192.168.100.17
                    }
                break;
            }
            break;
        case WM_CREATE:
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default:
            return(DefWindowProc(Window, Message, WParam, LParam));
    }
    return(0L);
}

void ConnectClient(char *User, char *IP, int Port)
{
    int Lines = 0;
    if(ClientSocketObject.Connect(IP, Port))
    {
        char Buffer[256];
        sprintf(Buffer, "User <%s>: Conected", User);
        SendMessage(hLB_Output, LB_ADDSTRING, 0, (LPARAM)Buffer);
        IsConected = true; 
    }
    else
    {
        char Buffer[256];
        sprintf(Buffer, "User: <%s> Could Not Conect To The Server", User);
        SendMessage(hLB_Output, LB_ADDSTRING, 0, (LPARAM)Buffer); 
        IsConected = false;
    }
    // Determine number of items in listbox
    Lines = SendMessage(hLB_Output, LB_GETCOUNT, 0, 0);
    // Flag last item as the selected item to scroll listbox down
    SendMessage(hLB_Output, LB_SETCURSEL, Lines-1, 0);
    // Unflag all items to eliminate negative highlight
    SendMessage(hLB_Output, LB_SETCURSEL, -1, 0);
}


DWORD WINAPI AsyncRecvFunc(LPVOID Param)
{
    ShearMemory *Memory = (ShearMemory *)Param;
    while(Memory->AppRunnign)
    {
        char Buffer[256];
        int BytesRecv;
        BytesRecv =  Memory->mClientSocketObject->Recv(Buffer, 256, 0);
        if(BytesRecv > 0)
        {
            int Lines = 0;
            SendMessage(*Memory->hLB_Output, LB_ADDSTRING, 0, (LPARAM)Buffer);
            // Determine number of items in listbox
            Lines = SendMessage(*Memory->hLB_Output, LB_GETCOUNT, 0, 0);
            // Flag last item as the selected item to scroll listbox down
            SendMessage(*Memory->hLB_Output, LB_SETCURSEL, Lines-1, 0);
            // Unflag all items to eliminate negative highlight
            SendMessage(*Memory->hLB_Output, LB_SETCURSEL, -1, 0);
        }
    }
    return 0;
}
