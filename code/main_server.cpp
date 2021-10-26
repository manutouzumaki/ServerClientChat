#include "socket.h"
#include <stdio.h>
#include <vector>

///////////////////////////////////////////////////////
// Server Program
///////////////////////////////////////////////////////

DWORD WINAPI ListenerThreadFunc(LPVOID Param);
DWORD WINAPI AsyncRecvFunc(LPVOID Param);

struct ShearMemory
{
    int AppRunnign;
    int ClientCount;
    SocketObject ServerSocketObject;
    SocketObject ClientSocketObjects[4];
    bool Initialize[4];
};

struct RecvMemory
{
    int AppRunnign;
    char Buffer[256];
    int BytesRecv;
    bool MemoryRecv;
    SocketObject *ClientSocketObject;
};

HANDLE Mutex = NULL;

int main(void)
{ 
    ShearMemory Memory;
    Memory.Initialize[0] = false;
    Memory.Initialize[1] = false;
    Memory.Initialize[2] = false;
    Memory.Initialize[3] = false;
    Memory.AppRunnign = 0;
    Memory.ClientCount = 0;
    RecvMemory recvMemory[4];
    for(int I = 0; I < 4; ++I)
    {
        recvMemory[I].BytesRecv = 0;
        recvMemory[I].MemoryRecv = false;
        recvMemory[I].AppRunnign = true;
    }


    LPDWORD ListenerThreadID = NULL;
    HANDLE ListenerThreadHandle;

    DWORD RecvThreadIDs[4];
    HANDLE RecvThreadHandles[4];

    // Create mutex
    Mutex = CreateMutex(NULL, FALSE, NULL);

    ListenerThreadHandle = CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)ListenerThreadFunc,
                                        (void *)&Memory, CREATE_SUSPENDED, ListenerThreadID);
    int Port = 7000; 
    // Attempt to bind
    if(Memory.ServerSocketObject.Bind(Port))
    {
        printf("<-- SOCKET BOUND -->\n");
        ResumeThread(ListenerThreadHandle);

        Memory.AppRunnign = true;

        while(Memory.AppRunnign)
        {    
            for(int I = 0;
                I < Memory.ClientCount;
                ++I)
            {
                if(!Memory.Initialize[I])
                {
                    recvMemory[I].ClientSocketObject = &Memory.ClientSocketObjects[I];
                    RecvThreadHandles[I] = CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)AsyncRecvFunc,
                                                       (void *)&recvMemory[I], CREATE_SUSPENDED, &RecvThreadIDs[I]);
                    ResumeThread(RecvThreadHandles[I]);
                    printf("New Chat thread Added\n");

                    Memory.Initialize[I] = true; 
                }
                 
                if(recvMemory[I].MemoryRecv == true)
                {
                    printf("<-- RECEIVED %d BYTES: %s -->\n", recvMemory[I].BytesRecv, recvMemory[I].Buffer);
                    recvMemory[I].MemoryRecv = false;
                    for(int J = 0; J < Memory.ClientCount; ++J)
                    {
                        int SendResult = Memory.ClientSocketObjects[J].Send(recvMemory[I].Buffer, 256, 0);
                        if(SendResult != SOCKET_ERROR)
                        {
                            printf("Message Send to User(%d): %s\n",J, recvMemory[I].Buffer);
                        }
                    }
                }

            } 
        }
        Memory.ServerSocketObject.Disconnect();
    }
    else
    {
        printf("**ERROR** Could Not Bind Socket\n"); 
    }

    for(int I = 0;
    I < Memory.ClientCount;
    ++I)
    {
        WaitForSingleObject(RecvThreadHandles[I], INFINITE);
        CloseHandle(RecvThreadHandles[I]);
    }

    WaitForSingleObject(ListenerThreadHandle, INFINITE);
    CloseHandle(ListenerThreadHandle);
    WaitForSingleObject(Mutex, INFINITE);
    CloseHandle(Mutex);

    WSACleanup();

    return 0;
}

DWORD WINAPI AsyncRecvFunc(LPVOID Param)
{
    RecvMemory *Memory = (RecvMemory *)Param;
    while(Memory->AppRunnign)
    {
        Memory->BytesRecv =  Memory->ClientSocketObject->Recv(Memory->Buffer, 256, 0);
        if(Memory->BytesRecv > 0)
        {
            Memory->MemoryRecv = true;
        }
    }
    return 0;
}

DWORD WINAPI ListenerThreadFunc(LPVOID Param)
{
    ShearMemory *Memory = (ShearMemory *)Param;
    
    while(Memory->AppRunnign)
    {
        if(Memory->ServerSocketObject.Listen() != SOCKET_ERROR)
        {
            printf("<-- SOCKET LISTENING -->\n"); 
            SocketObject ClientSocketObject;
            Memory->ClientSocketObjects[Memory->ClientCount] = ClientSocketObject; 
            if(Memory->ServerSocketObject.Accept(Memory->ClientSocketObjects[Memory->ClientCount]))
            {
                printf("<-- CLIENT ACCEPTED -->\n");
                ++Memory->ClientCount;
            }
            else
            {
                printf("**ERROR** Could Not Accept\n");
            }  
        }
        else
        {
            printf("**ERROR** Could Not Listen\n"); 
        }
    }

    return 0;

}
