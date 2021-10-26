SocketObject::SocketObject()
{
    WSADATA WsaData;
    WORD VersionRequested;

    VersionRequested = MAKEWORD(2, 0);
    Socket = INVALID_SOCKET;
    
    int Status = WSAStartup(VersionRequested, &WsaData);
}

SocketObject::~SocketObject()
{
    Disconnect();
}

int SocketObject::Bind(int Port)
{
    sockaddr_in ServerAddress = {};
    Socket = socket(AF_INET, SOCK_STREAM, 0);
    if(Socket == INVALID_SOCKET)
    {
        return 0;
    }
    
    ServerAddress.sin_family = AF_INET;
    ServerAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    ServerAddress.sin_port = htons(Port);
    
    if(bind(Socket, (sockaddr *)&ServerAddress, sizeof(sockaddr)) == SOCKET_ERROR)
    {
        Disconnect();
        return 0;
    }
    return 1;
}

void SocketObject::Disconnect()
{
    if(Socket != INVALID_SOCKET)
    {
        closesocket(Socket);
        Socket = INVALID_SOCKET;
    }
}

int SocketObject::Listen()
{
    return listen(Socket, 32);
}

bool SocketObject::Accept(SocketObject &AcceptSocket)
{
    sockaddr_in ClientAddress;
    int ClientSize = sizeof(sockaddr_in);

    AcceptSocket.Socket = accept(Socket, (sockaddr *)&ClientAddress, &ClientSize);
    if(AcceptSocket.Socket == INVALID_SOCKET)
    {
        return false;
    }
    return true;
}

bool SocketObject::Connect(char *ServerAddress, int Port)
{
    sockaddr_in ServAddr = {};
    LPHOSTENT Host;
    int Err;

    // Open the Socket
    Socket = socket(AF_INET, SOCK_STREAM, 0);
    if(Socket == INVALID_SOCKET)
    {
        return false;
    }
    ServAddr.sin_family = AF_INET;
    ServAddr.sin_addr.s_addr = inet_addr(ServerAddress);
    if(ServAddr.sin_addr.s_addr == INADDR_NONE)
    {
        Host = gethostbyname(ServerAddress);
        if(Host != NULL)
        {
            ServAddr.sin_addr.s_addr = ((LPIN_ADDR)Host->h_addr)->s_addr;
        }
        else
        {
            return false;
        }
    }
    // Assign the Port
    ServAddr.sin_port = htons(Port);
    // Establish Connection
    Err = connect(Socket, (sockaddr *)&ServAddr, sizeof(sockaddr));
    if(Err == SOCKET_ERROR)
    {
        Disconnect();
        return false;
    }
    return true;
}

int SocketObject::Send(char *Buffer, int BufferLen, int Flags)
{
    return send(Socket, Buffer, BufferLen, Flags);
}


int SocketObject::Recv(char *Buffer, int BufferLen, int Flags)
{
    return recv(Socket, Buffer, BufferLen, Flags); 
}
