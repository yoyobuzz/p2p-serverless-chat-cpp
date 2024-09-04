#include "utils.h"
#include "colors.h"

thread t1;
thread t2;
string MyName;

// Mutex lock for stdout
mutex cout_mtx;

// BroadCast to all Peers obtained from server
void BroadCast(const vector<PeerData> &peers, char *msg, string Name)
{
    for (auto p : peers)
    {
        struct sockaddr_in server;
        server.sin_family = AF_INET;
        server.sin_port = htons(stoi(p.port));
        if (inet_pton(AF_INET, p.IP.c_str(), &(server.sin_addr)) <= 0)
        {
            cerr << "Invalid IP address of a client" << endl;
        }
        socklen_t len = sizeof(struct sockaddr_in);
        int newSock = socket(AF_INET, SOCK_STREAM, 0);
        if (connect(newSock, (struct sockaddr *)&server, len) < 0)
        {
            cerr << "Connection Error inside BroadCast, check Client PORT!!" << endl;
        }
        else
        {
            string res = "*" + Name + ": " + string(msg);
            send(newSock, res.c_str(), res.size() + 1, 0);
        }
        close(newSock);
    }
}

void DM(string port, string IP, string Name, char *msg)
{
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(stoi(port));
    if (inet_pton(AF_INET, IP.c_str(), &(server.sin_addr)) <= 0)
    {
        cerr << "Invalid IP address of a client";
    }
    socklen_t len = sizeof(struct sockaddr_in);
    int newSock = socket(AF_INET, SOCK_STREAM, 0);
    if (connect(newSock, (struct sockaddr *)&server, len) < 0)
    {
        cerr << "Connection Error inside DM, check Client PORT!!";
    }
    else
    {
        string res = Name + " [DM]: " + string(msg);
        send(newSock, res.c_str(), res.size() + 1, 0);
    }
    close(newSock);
}

void ManagePeers(int sock)
{
    char primessage[LEN];
    bzero(&primessage, sizeof(primessage));
    int recLen = recv(sock, primessage, sizeof(primessage), 0);
    primessage[recLen] = '\0';
    bool isDM = false;
    cout_mtx.lock();
    Clear(8);
    if (primessage[0] == '*')
    {
        strcpy(primessage, &primessage[1]);
        isDM = true;
    }
    cout << (!isDM ? BLUE : MAGENTA) << primessage << RESET << endl;
    cout << YELLOW << "You: " << RESET;
    fflush(stdout);
    cout_mtx.unlock();

    close(sock);
}

void Clientside(int sock)
{
    char message[LEN];
    bzero(&message, sizeof(message));
    while (1)
    {
        cout_mtx.lock();
        Clear(8);
        cout << YELLOW << "You: " << RESET;
        fflush(stdout);
        cout_mtx.unlock();

        bzero(&message, sizeof(message));

        // cout_mtx.lock();
        cin.getline(message, LEN);

        send(sock, message, sizeof(message), 0);
        // cout_mtx.unlock();

        if (strcmp(message, exitCmd) == 0)
        {
            break;
        }

        bzero(&message, sizeof(message));
        int servlen = read(sock, message, LEN);
        message[servlen] = '\0';

        if (strncmp(message, ServerBroadCast, strlen(ServerBroadCast)) == 0)
        {
            // BroadCast ##:Name-Port-IP\n format
            vector<PeerData> Peers = getPeerData(message);

            cout_mtx.lock();
            cout << WHITE << "Enter your BroadCast Message: " << RESET;
            fflush(stdout);
            cout_mtx.unlock();

            char broadmsg[LEN];
            bzero(&broadmsg, sizeof(broadmsg));

            cout_mtx.lock();
            cin.getline(broadmsg, LEN);
            cout_mtx.unlock();

            thread BroadcastThread(BroadCast, Peers, broadmsg, MyName);
            if (BroadcastThread.joinable())
                BroadcastThread.join();
        }
        else if (strncmp(message, ServerDM, strlen(ServerDM)) == 0)
        {
            // DM **:Name-Port-IP\n format
            vector<PeerData> Peers = getPeerData(message);

            cout_mtx.lock();
            if (Peers.size() == 0)
            {
                cout << RED << "No clients found to DM..." << RESET << endl;
                cout_mtx.unlock();
                continue;
            }
            cout << YELLOW << "The list of the clients connected are: " << RESET << endl;
            for (int i = 0; i < Peers.size(); i++)
            {
                cout << colours[(i + 1) % 5] << i + 1 << ". " << Peers[i] << RESET << endl;
            }
            cout << WHITE << "Enter the number of the client you want to DM: " << RESET;
            fflush(stdout);
            char DMmsg[LEN];
            bzero(&DMmsg, sizeof(DMmsg));
            int num;
            cin.getline(DMmsg, LEN);
            num = atoi(DMmsg);
            if (num > Peers.size())
            {
                cout << RED << "Not a valid number. Try again..." << RESET << endl;
                cout_mtx.unlock();
            }
            else
            {
                bzero(&DMmsg, sizeof(DMmsg));
                cout << WHITE << "Enter the DM Message: ";
                fflush(stdout);
                cin.getline(DMmsg, LEN);
                cout << RESET;
                cout_mtx.unlock();
                string _p = Peers[num - 1].port;
                string _IP = Peers[num - 1].IP;
                thread DMThread(DM, _p, _IP, MyName, DMmsg);
                if (DMThread.joinable())
                    DMThread.join();
            }
        }
        else if (servlen > 0)
        {
            cout << RED << message << RESET << endl;
            if (strcmp(message, serverExit) == 0)
            {
                break;
            }
        }
        else if (servlen == 0)
        {
            cout << RED << "Server closed unexpectedly..." << RESET << endl;
            break;
        }
    }
    if (t2.joinable())
        t2.detach();
}

void Serverside(int servport)
{
    // Server Side
    int servSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (servSocket < 0)
    {
        error("[Peer server] Socket Creation Failed!!");
    }

    struct sockaddr_in servAddr;
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = INADDR_ANY;
    servAddr.sin_port = htons(servport);

    if (bind(servSocket, (struct sockaddr *)&servAddr, sizeof(servAddr)))
    {
        error("[Peer server] PORT Busy, try with different PORT!!");
    }

    if (listen(servSocket, MAX_CLIENTS) < 0)
    {
        error("[Peer server] Listen failure...");
    }

    struct sockaddr_in cliAddr;
    socklen_t clilen;
    clilen = sizeof(cliAddr);

    while (1)
    {
        int incSocket = accept(servSocket, (struct sockaddr *)&cliAddr, &clilen);
        if (incSocket < 0)
        {
            error("[Peer server] New Socket Accept Failure!!");
        }
        else
        {
            thread peer(ManagePeers, incSocket);
            if (peer.joinable())
                peer.join();
        }
    }
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        error("Usage : ./client <IP Address> <Port Name>");
    }

    int port = atoi(argv[2]);
    int servport;

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    socklen_t len = sizeof(struct sockaddr_in);

    char message[LEN];
    int sock = socket(AF_INET, SOCK_STREAM, 0);

    if (sock < 0)
    {
        error("Client Socket Failure!!");
    }

    if (inet_aton(argv[1], &server.sin_addr) == 0)
    {
        error("IP address error!!");
    }

    if (connect(sock, (struct sockaddr *)&server, len) < 0)
    {
        error("Connection Error, check server PORT!!");
    }
    else
    {
        char name[LEN];

        cout_mtx.lock();
        cout << WHITE << "What is your name? ";
        fflush(stdout);

        bzero(&name, sizeof(name));

        cin.getline(name, LEN);
        cout << RESET;

        MyName = string(name);
        send(sock, name, sizeof(name), 0);

        char newbuff[LEN];
        int newlen = read(sock, newbuff, LEN);
        newbuff[newlen] = '\0';

        // receive the new port for starting peer-server from main server
        servport = atoi(newbuff);

        cout << GREEN << "---- Welcome to Chat Room ----" << RESET << endl;
        cout_mtx.unlock();
    }

    t1 = thread(Clientside, sock);
    t2 = thread(Serverside, servport);

    if (t1.joinable())
        t1.join();
    if (t2.joinable())
        t2.join();

    return 0;
}