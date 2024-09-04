#include <iostream>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <vector>
#include <mutex>
#include <stdlib.h>
#include <unistd.h>
#include <thread>
#include <ctime>
#include <sys/time.h>
#include <vector>
#include <string>
#include <sstream>
using namespace std;

#define LEN 5000
#define MAX_CLIENTS 6
char exitCmd[] = "/exit";
char BcCmd[] = "/broadcast";
char ServerBroadCast[] = "##:";
char ServerDM[] = "**:";
char serverExit[] = "Server Exit";
char broadcastCmd[] = "/broadcast";
char dmCmd[] = "/DM";
char nothing[] = "Use /broadcast for Sending to Everyone and /DM for Private Message";

// PeerData class for clients to get the Peers parsed from server response
class PeerData
{
public:
    string name;
    string port;
    string IP;

    friend ostream &operator<<(ostream &os, const PeerData &data)
    {
        os << "Name: " << data.name << ", Port: " << data.port << ", IP: " << data.IP;
        return os;
    }
};

void error(const string &msg)
{
    cerr << msg << endl;
    exit(2);
}

vector<PeerData> getPeerData(char *res)
{
    char *response = res + 3; // To remove the custom header
    stringstream ss(response);
    string line;

    vector<PeerData> peers;

    while (getline(ss, line, '\n'))
    {
        stringstream lineStream(line);
        string token;

        PeerData data;
        int count = 0;
        while (getline(lineStream, token, '-'))
        {
            if (count == 0)
            {
                data.name = token;
            }
            else if (count == 1)
            {
                data.port = token;
            }
            else if (count == 2)
            {
                data.IP = token;
            }
            count++;
        }

        peers.push_back(data);
    }

    return peers;
}

// To clear the stdout
void Clear(int num)
{
    char backSpace = 8;
    for (int i = 0; i < num; i++)
    {
        cout << backSpace;
    }
}
