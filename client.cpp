#include <iostream>
#include <string>
#include <thread>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

using namespace std;

int sock;
bool active = true;

void receive_loop() {
    char buf[1024];
    while (active) {
        int n = recv(sock, buf, 1024, 0);
        if (n <= 0) {
            active = false;
            exit(0);
        }
        buf[n] = '\0';
        cout << "\n" << buf << "\n> " << flush;
    }
}

void heartbeat_loop(string name) {
    int udp_sock = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in serv;
    serv.sin_family = AF_INET;
    serv.sin_port = htons(8889);
    inet_pton(AF_INET, "127.0.0.1", &serv.sin_addr);
    
    string msg = "Alive:" + name;
    
    while (active) {
        sendto(udp_sock, msg.c_str(), msg.length(), 0, (struct sockaddr*)&serv, sizeof(serv));
        this_thread::sleep_for(chrono::seconds(10));
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cout << "Usage: ./client <CampusName>" << endl;
        return 0;
    }
    
    string name = argv[1];
    
    sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serv;
    serv.sin_family = AF_INET;
    serv.sin_port = htons(8888);
    inet_pton(AF_INET, "127.0.0.1", &serv.sin_addr);
    
    if (connect(sock, (struct sockaddr*)&serv, sizeof(serv)) < 0) {
        cout << "Connection failed" << endl;
        return -1;
    }
    
    string auth = "Campus:" + name;
    send(sock, auth.c_str(), auth.length(), 0);
    
    thread r(receive_loop);
    r.detach();
    
    thread h(heartbeat_loop, name);
    h.detach();
    
    string input;
    cout << "Connected. Format: To:Target,Msg:Text" << endl;
    cout << "> " << flush;
    
    while (active) {
        getline(cin, input);
        if (input == "exit") break;
        send(sock, input.c_str(), input.length(), 0);
        cout << "> " << flush;
    }
    
    close(sock);
    return 0;
}
