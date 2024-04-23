#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <mutex>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAX_LEN 200
#define NUM_COLORS 6

using namespace std;

struct Terminal {
    int id;
    string name;
    int socket;
    thread th;
};

std::vector<Terminal> clients;
std::string def_col="\033[0m";
std::string colors[] = {"\033[31m", "\033[32m", "\033[33m", "\033[34m", "\033[35m", "\033[36m"};
int seed = 0;
std::mutex cout_mtx;
std::mutex clients_mtx;

string color(int code);
void set_name(int id, const char name[]);
void shared_print(const string& str, bool endLine=true);
void broadcast_message(const string& message, int sender_id);
void broadcast_message(int num, int sender_id);
void end_connection(int id);
void handle_client(int client_socket, int id);

int main() {
    int server_socket;
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket: ");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(8888);
    server.sin_addr.s_addr = INADDR_ANY;
    memset(&server.sin_zero, 0, sizeof(server.sin_zero));

    if (bind(server_socket, reinterpret_cast<struct sockaddr *>(&server), sizeof(struct sockaddr_in)) == -1) {
        perror("bind error: ");
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket, 8) == -1) {
        perror("listen error: ");
        exit(EXIT_FAILURE);
    }

    cout << colors[NUM_COLORS - 1] << "\n\t  <><><>Welcome To BestSec's Socket Chat Application!<><><>   " << endl << def_col;
    

    while (true) {
        struct sockaddr_in client;
        int client_socket;
        unsigned int len = sizeof(sockaddr_in);

        if ((client_socket = accept(server_socket, reinterpret_cast<struct sockaddr *>(&client), &len)) == -1) {
            perror("accept error: ");
            exit(EXIT_FAILURE);
        }
        seed++;
        thread t(handle_client, client_socket, seed);
        lock_guard<mutex> guard(clients_mtx);
        clients.push_back({seed, "Anonymous", client_socket, move(t)});
    }

    for (auto& client : clients) {
        if (client.th.joinable())
            client.th.join();
    }

    close(server_socket);
    return 0;
}

string color(int code) {
    return colors[code % NUM_COLORS];
}

void set_name(int id, const char name[]) {
    for (auto& client : clients) {
        if (client.id == id) {
            client.name = name;
            break;
        }
    }
}

void shared_print(const string& str, bool endLine) {
    lock_guard<mutex> guard(cout_mtx);
    cout << str;
    if (endLine)
        cout << endl;
}

void broadcast_message(const string& message, int sender_id) {
    char temp[MAX_LEN];
    strcpy(temp, message.c_str());
    for (const auto& client : clients) {
        if (client.id != sender_id) {
            send(client.socket, temp, sizeof(temp), 0);
        }
    }
}

void broadcast_message(int num, int sender_id) {
    for (const auto& client : clients) {
        if (client.id != sender_id) {
            send(client.socket, &num, sizeof(num), 0);
        }
    }
}

void end_connection(int id) {
    for (auto it = clients.begin(); it != clients.end(); ++it) {
        if (it->id == id) {
            lock_guard<mutex> guard(clients_mtx);
            it->th.detach();
            close(it->socket);
            clients.erase(it);
            break;
        }
    }
}

void handle_client(int client_socket, int id) {
    char name[MAX_LEN], str[MAX_LEN];
    recv(client_socket, name, sizeof(name), 0);
    set_name(id, name);

    // Display welcome message
    string welcome_message = name;
    welcome_message += " has joined";
    broadcast_message("#NULL", id);
    broadcast_message(id, id);
    broadcast_message(welcome_message, id);
    shared_print(color(id) + welcome_message + def_col);

    while (true) {
        int bytes_received = recv(client_socket, str, sizeof(str), 0);
        if (bytes_received <= 0)
            break;
        if (strcmp(str, "#exit") == 0) {
            // Display leaving message
            string message = name;
            message += " has left";
            broadcast_message("#NULL", id);
            broadcast_message(id, id);
            broadcast_message(message, id);
            shared_print(color(id) + message + def_col);
            end_connection(id);
            break;
        }
        
        // Decrypt incoming message using Caesar cipher with key 3

        
        broadcast_message(name, id);
        broadcast_message(id, id);
        broadcast_message(str, id);
        shared_print(color(id) + name + " : " + def_col + str);
    }
}
