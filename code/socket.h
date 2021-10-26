#ifndef SOCKET_H
#define SOCKET_H

#include <windows.h>

class SocketObject
{
    private:
    public:
        SOCKET Socket;
        SocketObject();
        ~SocketObject();
        int Bind(int Port);
        void Disconnect();
        int Listen();
        bool Accept(SocketObject& AcceptSocket);
        bool Connect(char *ServerAddress, int Port);
        int Send(char *Buffer, int BufferLen, int Flags);
        int Recv(char *Buffer, int BufferLen, int Flags);
};

#include "socket.cpp"

#endif
