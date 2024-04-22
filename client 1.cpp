#include <iostream>
#include <cstring>
#include <thread>
#include <signal.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fstream> // For file operations

#define MAX_LEN 200
#define NUM_COLORS 6

using namespace std;

bool exit_flag = false;
thread t_send, t_recv;
int client_socket;
string def_col = "\033[0m";
string colors[] = {"\033[31m", "\033[32m", "\033[33m", "\033[34m", "\033[35m", "\033[36m"};

void catch_ctrl_c(int signal);
string color(int code);
int eraseText(int cnt);
void send_message(int client_socket);
void recv_message(int client_socket);
bool signup(const string& username, const string& password);

bool signup(const string& username, const string& password) {
    // Open the user credentials file in append mode
    ofstream file("user.txt", ios::app);
    if (!file) {
        cerr << "Unable to open file." << endl;
        return false;
    }

    // Write the new user credentials to the file
    file << username << " " << password << endl;

    return true;
}

int main() {
    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket: ");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in client;
    client.sin_family = AF_INET;
    client.sin_port = htons(8888); // Port no. of server
    client.sin_addr.s_addr = INADDR_ANY;
    //client.sin_addr.s_addr = inet_addr("127.0.0.1"); // Provide IP address of server
    memset(&client.sin_zero, 0, sizeof(client.sin_zero));

    if (connect(client_socket, reinterpret_cast<struct sockaddr *>(&client), sizeof(struct sockaddr_in)) == -1) {
        perror("connect: ");
        exit(EXIT_FAILURE);
    }
    signal(SIGINT, catch_ctrl_c);

    char choice;
    cout << "Do you want to sign up (s) or login (l)? ";
    cin >> choice;
    cin.ignore(); // Consume newline character

    char username[MAX_LEN];
    char password[MAX_LEN];

    if (choice == 's') {
        cout << "Enter your desired username: ";
        cin >> username;
        cout << "Enter your desired password: ";
        cin >> password;

        if (!signup(username, password)) {
            cerr << "Failed to sign up. Exiting..." << endl;
            close(client_socket);
            exit(EXIT_FAILURE);
        }

        cout << "Sign up successful. Please log in." << endl;

        // Prompt user to log in after successful registration
        cout << "Enter your username: ";
        cin >> username;
        cout << "Enter your password: ";
        cin >> password;
        send(client_socket, username, strlen(username), 0);
        send(client_socket, password, strlen(password), 0);
    } else if (choice == 'l') {
        cout << "Enter your username: ";
        cin >> username;
        cout << "Enter your password: ";
        cin >> password;
        cout << "Login Successful";

        send(client_socket, username, strlen(username), 0);
        send(client_socket, password, strlen(password), 0);
    } else {
        cout << "Invalid choice. Exiting..." << endl;
        close(client_socket);
        exit(EXIT_FAILURE);
    }


    send(client_socket, username, strlen(username), 0);
    send(client_socket, password, strlen(password), 0);

    cout << colors[NUM_COLORS - 1] << "\n\t  ====== Welcome to the chat-room ======   " << endl << def_col;

    thread t1(send_message, client_socket);
    thread t2(recv_message, client_socket);

    t_send = move(t1);
    t_recv = move(t2);

    if (t_send.joinable())
        t_send.join();
    if (t_recv.joinable())
        t_recv.join();

    return 0;
}

void catch_ctrl_c(int signal) {
    char str[MAX_LEN] = "#exit";
    send(client_socket, str, strlen(str), 0);
    exit_flag = true;
    t_send.detach();
    t_recv.detach();
    close(client_socket);
    exit(signal);
}

string color(int code) {
    return colors[code % NUM_COLORS];
}

int eraseText(int cnt) {
    char back_space = 8;
    for (int i = 0; i < cnt; i++) {
        cout << back_space;
    }
    return 0;
}

void send_message(int client_socket) {
    while (true) {
        cout << colors[1] << "You : " << def_col;
        char str[MAX_LEN];
        cin.getline(str, MAX_LEN);
        send(client_socket, str, strlen(str), 0);
        if (strcmp(str, "#exit") == 0) {
            exit_flag = true;
            t_recv.detach();
            close(client_socket);
            return;
        }
    }
}

void recv_message(int client_socket) {
    while (true) {
        if (exit_flag)
            return;
        char username[MAX_LEN], str[MAX_LEN];
        int color_code;
        int bytes_received = recv(client_socket, username, sizeof(username), 0);
        if (bytes_received <= 0)
            continue;
        recv(client_socket, &color_code, sizeof(color_code), 0);
        recv(client_socket, str, sizeof(str), 0);
        eraseText(6);
        if (strcmp(username, "#NULL") != 0)
            cout << color(color_code) << username << " : " << def_col << str << endl;
        else
            cout << color(color_code) << str << endl;
        cout << colors[1] << "You : " << def_col;
        fflush(stdout);
    }
}
