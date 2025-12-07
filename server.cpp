#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <thread>
#include <mutex>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ctime>

using namespace std;

map<string, int> clients;
map<string, time_t> heartbeats;
mutex mtx;

void broadcast_msg(string msg) {
    lock_guard<mutex> lock(mtx);
    for (auto const& pair : clients) {
        send(pair.second, msg.c_str(), msg.length(), 0);
    }
}

void handle_client(int sock) {
    char buf[1024];
    string name = "";
    
    int n = recv(sock, buf, 1024, 0);
    if (n > 0) {
        buf[n] = '\0';
        string s(buf);
        if (s.find("Campus:") != string::npos) {
            name = s.substr(7);
            mtx.lock();
            clients[name] = sock;
            heartbeats[name] = time(nullptr);
            mtx.unlock();
            string w = "Welcome to Server\n";
            send(sock, w.c_str(), w.length(), 0);
            cout << name << " connected." << endl;
        } else {
            close(sock);
            return;
        }
    }

    while (true) {
        memset(buf, 0, 1024);
        n = recv(sock, buf, 1024, 0);
        if (n <= 0) {
            mtx.lock();
            clients.erase(name);
            heartbeats.erase(name);
            mtx.unlock();
            close(sock);
            cout << name << " disconnected." << endl;
            break;
        }
        string msg(buf);
        size_t p1 = msg.find("To:");
        size_t p2 = msg.find(",Msg:");
        
        if (p1 != string::npos && p2 != string::npos) {
            string target = msg.substr(p1 + 3, p2 - (p1 + 3));
            string content = msg.substr(p2 + 5);
            
            mtx.lock();
            if (clients.count(target)) {
                string fwd = "From " + name + ": " + content;
                send(clients[target], fwd.c_str(), fwd.length(), 0);
                cout << "Routed " << name << " -> " << target << endl;
            }
            mtx.unlock();
        }
    }
}

void udp_recv() {
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in serv, cli;
    serv.sin_family = AF_INET;
    serv.sin_addr.s_addr = INADDR_ANY;
    serv.sin_port = htons(8889);
    
    bind(sockfd, (struct sockaddr*)&serv, sizeof(serv));
    
    char buf[1024];
    socklen_t len = sizeof(cli);
    
    while(true) {
        int n = recvfrom(sockfd, buf, 1024, 0, (struct sockaddr*)&cli, &len);
        if (n > 0) {
            buf[n] = '\0';
            string s(buf);
            if (s.find("Alive:") != string::npos) {
                string who = s.substr(6);
                mtx.lock();
                heartbeats[who] = time(nullptr);
                mtx.unlock();
            }
        }
    }
}

void admin_menu() {
    string cmd;
    while(true) {
        getline(cin, cmd);
        if (cmd == "status") {
            mtx.lock();
            time_t now = time(nullptr);
            for (auto const& pair : heartbeats) {
                double diff = difftime(now, pair.second);
                cout << pair.first << ": " << (diff < 15 ? "Online" : "Offline") << endl;
            }
            mtx.unlock();
        } else if (cmd.substr(0, 4) == "cast") {
            broadcast_msg("ADMIN: " + cmd.substr(5));
        }
    }
}

int main() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8888);
    
    bind(server_fd, (struct sockaddr*)&address, sizeof(address));
    listen(server_fd, 10);
    
    cout << "Server started on 8888 (TCP) and 8889 (UDP)" << endl;
    
    thread t_udp(udp_recv);
    t_udp.detach();
    
    thread t_admin(admin_menu);
    t_admin.detach();
    
    while (true) {
        int new_socket = accept(server_fd, nullptr, nullptr);
        thread t(handle_client, new_socket);
        t.detach();
    }
    return 0;
}
