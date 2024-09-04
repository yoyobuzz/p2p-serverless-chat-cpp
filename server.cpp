#include "utils.h"
#include "colors.h"
// g++ server.cpp -lpthread -o server

bool isRunning = 1;
class Client
{
public:
    int id;
    string port;
    string IP;
    string name;
    int sock;
    thread th;
};

vector<Client> client_sockets;

int server;

int setup_server(char *port)
{
    int portno = atoi(port);

    // Open socket
    int servSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (servSocket < 0)
    {
        error("Server Socket Creation Failed!!");
    }

    struct sockaddr_in servAddr;

    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = INADDR_ANY;
    servAddr.sin_port = htons(portno);

    // bind to port
    if (bind(servSocket, (struct sockaddr *)&servAddr, sizeof(servAddr)))
    {
        error("PORT Busy, try with different PORT!!");
    }

    if (listen(servSocket, MAX_CLIENTS) < 0)
    {
        error("Server Listening Failed!!");
    }

    cout << WHITE << "Server listening on port: " << port << RESET << endl;

    return servSocket;
}

void CloseConn(int id)
{
    for (int i = 0; i < client_sockets.size(); i++)
    {
        if (client_sockets[i].id == id)
        {
            if (client_sockets[i].th.joinable())
                client_sockets[i].th.detach();
            close(client_sockets[i].sock);
            client_sockets.erase(client_sockets.begin() + i);
            break;
        }
    }
}

void BroadCast(string Msg, int id)
{
    for (int i = 0; i < client_sockets.size(); i++)
    {
        if (client_sockets[i].id != id) // Send to all except sender
        {
            send(client_sockets[i].sock, Msg.c_str(), Msg.size() + 1, 0);
        }
    }
}

void BroadCast(string Msg) // Broadcast to all
{
    for (int i = 0; i < client_sockets.size(); i++)
    {
        send(client_sockets[i].sock, Msg.c_str(), Msg.size() + 1, 0);
    }
}

void HandleExit(thread &other)
{
    char exitserver[LEN];
    while (1)
    {
        bzero(&exitserver, sizeof(exitserver));
        cin.getline(exitserver, LEN);
        if (strcmp(exitserver, exitCmd) == 0)
        {
            cout << RED << "Server Exiting..." << RESET << endl;
            BroadCast(string(serverExit));
            isRunning = false;
            break;
        }
    }
}

void ClientThread(int sock, int id, int port)
{
    srand(time(NULL));

    int newPort = port + rand() % 1000;
    if (newPort > 65535)
    {
        newPort -= 1000;
    }

    char name[LEN];
    bzero(&name, sizeof(name));
    int valRead = read(sock, name, LEN);
    name[valRead] = '\0';

    // Set Name of Client
    for (int i = 0; i < client_sockets.size(); i++)
    {
        if (client_sockets[i].id == id)
        {
            client_sockets[i].name = string(name);
            client_sockets[i].port = to_string(newPort);
            break;
        }
    }

    // Welcome Message in server
    cout << GREEN << name << " "
         << "has joined..." << RESET << endl;

    // Sending the port name to the client to start its server side...
    string bufff = to_string(newPort);
    send(sock, bufff.c_str(), bufff.size() + 1, 0);

    char message[LEN];

    while (1)
    {
        // Attempt to read from stdin (non-blocking)
        bzero(&message, sizeof(message));
        int recLen = recv(sock, message, sizeof(message), 0);
        message[recLen] = '\0';
        // cout << message << endl;
        if (recLen <= 0 || strcmp(message, exitCmd) == 0) // recLen<=0
        {
            cout << RED << name << " "
                 << "has left the chat..." << RESET << endl;
            break;
        }
        else if (strcmp(message, broadcastCmd) == 0)
        {
            string peerDataString = string(ServerBroadCast);
            for (int i = 0; i < client_sockets.size(); i++)
            {
                if (client_sockets[i].id != id)
                {
                    peerDataString.append(client_sockets[i].name + "-" + client_sockets[i].port + "-" + client_sockets[i].IP + "\n");
                }
            }
            if (peerDataString.length() >= 4)
                peerDataString.pop_back();
            send(sock, peerDataString.c_str(), peerDataString.size() + 1, 0);
        }
        else if (strcmp(message, dmCmd) == 0)
        {
            string peerDataString = string(ServerDM);
            for (int i = 0; i < client_sockets.size(); i++)
            {
                if (client_sockets[i].id != id)
                {
                    peerDataString.append(client_sockets[i].name + "-" + client_sockets[i].port + "-" + client_sockets[i].IP + "\n");
                }
            }
            if (peerDataString.length() >= 4)
                peerDataString.pop_back();
            send(sock, peerDataString.c_str(), peerDataString.size() + 1, 0);
        }
        else
        {
            send(sock, nothing, sizeof(nothing), 0);
        }
    }

    CloseConn(id);
}

void HandleClient(struct sockaddr_in &cliAddr, socklen_t &clilen, int &CliID)
{
    while (isRunning)
    {
        int newSocket = accept(server, (struct sockaddr *)&cliAddr, &clilen);
        if (newSocket < 0)
        {
            return;
        }

        cout << WHITE << "New Connection established with Socket # " << newSocket << " , IP : " << inet_ntoa(cliAddr.sin_addr) << " , PORT: " << ntohs(cliAddr.sin_port) << RESET << endl;

        thread cliThread(ClientThread, newSocket, CliID, ntohs(cliAddr.sin_port));

        client_sockets.push_back({CliID, "8080", inet_ntoa(cliAddr.sin_addr), string("NewClient"), newSocket, (move(cliThread))});

        CliID++;
    }
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        error("Usage : ./server <port_number>");
    }

    server = setup_server(argv[1]);

    cout << YELLOW << "----- ChatRoom Server -----" << RESET << endl;

    struct sockaddr_in cliAddr;
    socklen_t clilen;
    clilen = sizeof(cliAddr);

    int CliID = 0;

    thread t1, t2;

    t1 = thread(HandleClient, ref(cliAddr), ref(clilen), ref(CliID));
    t2 = thread(HandleExit, ref(t1));

    if (t2.joinable())
    {
        t2.join();
        if (t1.joinable())
            t1.detach();
    }

    if (t1.joinable())
        t1.detach();

    for (int i = 0; i < client_sockets.size(); i++)
    {
        if (client_sockets[i].th.joinable())
            client_sockets[i].th.detach();
    }
    if (shutdown(server, SHUT_RDWR) < 0)
    {
        error("Server closing error!!");
    }
    return 0;
}