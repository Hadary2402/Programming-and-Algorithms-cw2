#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip> 
#include <cstring>
#include <thread>
#include <signal.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <openssl/evp.h>
#include <openssl/sha.h>

#define MAX_LEN 200
#define NUM_COLORS 6

using namespace std;

// Global variables
char name[MAX_LEN];
bool exit_flag = false;
thread t_send, t_recv;
int client_socket;
string def_col = "\033[0m";
string colors[] = {"\033[31m", "\033[32m", "\033[33m", "\033[34m", "\033[35m", "\033[36m"};

// Function prototypes
void catch_ctrl_c(int signal);
string color(int code);
int eraseText(int cnt);
void send_message(int client_socket);
void recv_message(int client_socket);
bool signup();
bool login();
string hashPassword(const string &password);
void writeUserData(const string &name, const string &hashedPassword);
bool readUserData(const string &name, string &hashedPassword);

// Hashes the password using SHA-256
string hashPassword(const string &password) {
    EVP_MD_CTX *mdctx;
    const EVP_MD *md;
    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int hashLen;

    md = EVP_sha256();
    mdctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(mdctx, md, NULL);
    EVP_DigestUpdate(mdctx, password.c_str(), password.size());
    EVP_DigestFinal_ex(mdctx, hash, &hashLen);
    EVP_MD_CTX_free(mdctx);

    stringstream ss;
    for (unsigned int i = 0; i < hashLen; i++) {
        ss << hex << setw(2) << setfill('0') << (int)hash[i]; 
    }
    return ss.str();
}

// Writes user data to a file
void writeUserData(const string &name, const string &hashedPassword) {
    ofstream outfile("userdata.txt", ios::app);
    if (outfile.is_open()) {
        outfile << name << " " << hashedPassword << endl;
        outfile.close();
    } else {
        cerr << "Unable to open file for writing userdata." << endl;
    }
}

// Reads user data from a file
bool readUserData(const string &name, string &hashedPassword) {
    ifstream infile("userdata.txt");
    string line;
    while (getline(infile, line)) {
        istringstream iss(line);
        string user, hashedPass;
        if (iss >> user >> hashedPass) {
            if (user == name) {
                hashedPassword = hashedPass;
                return true;
            }
        }
    }
    return false;
}

// Main function
int main() {
    // Creating a socket
    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket: ");
        exit(EXIT_FAILURE);
    }

    // Configuring server address
    struct sockaddr_in client;
    client.sin_family = AF_INET;
    client.sin_port = htons(8888); // Port no. of server
    client.sin_addr.s_addr = INADDR_ANY;
    memset(&client.sin_zero, 0, sizeof(client.sin_zero));

    // Connecting to server
    if (connect(client_socket, reinterpret_cast<struct sockaddr *>(&client), sizeof(struct sockaddr_in)) == -1) {
        perror("connect: ");
        exit(EXIT_FAILURE);
    }
    signal(SIGINT, catch_ctrl_c);

    // Authentication loop
    bool loggedIn = false;
    while (!loggedIn) {
        cout << "Choose an option:\n1. Sign Up\n2. Log In\nYour choice: ";
        int choice;
        cin >> choice;
        cin.ignore(); 
        switch (choice) {
            case 1:
                if (signup()) {
                    cout << "Signup successful!\n";
                } else {
                    cout << "Signup failed!\n";
                }
                break;
            case 2:
                if (login()) {
                    loggedIn = true;
                } else {
                    cout << "Login failed!\n";
                }
                break;
            default:
                cout << "Invalid choice!\n";
        }
    }

    // Starting send and receive threads
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

// Signal handler for Ctrl+C
void catch_ctrl_c(int signal) {
    char str[MAX_LEN] = "#exit";
    send(client_socket, str, sizeof(str), 0);
    exit_flag = true;
    t_send.detach();
    t_recv.detach();
    close(client_socket);
    exit(signal);
}

// Returns ANSI color escape sequence
string color(int code) {
    return colors[code % NUM_COLORS];
}

// Erases text from the console
int eraseText(int cnt) {
    char back_space = 8;
    for (int i = 0; i < cnt; i++) {
        cout << back_space;
    }
    return 0;
}

// Sends messages to the server
void send_message(int client_socket) {
    while (true) {
        cout << colors[1] << "You : " << def_col;
        char str[MAX_LEN];
        cin.getline(str, MAX_LEN);
        
        // Encrypting the message
        for (int i = 0; str[i] != '\0'; ++i) {
            if (isalpha(str[i])) {
                str[i] = (str[i] + 3 - 'a') % 26 + 'a';
            }
        }
        
        send(client_socket, str, sizeof(str), 0);
        if (strcmp(str, "#exit") == 0) {
            exit_flag = true;
            t_recv.detach();
            close(client_socket);
            return;
        }
    }
}

// Receives messages from the server
void recv_message(int client_socket) {
    while (true) {
        if (exit_flag)
            return;
        char name[MAX_LEN], str[MAX_LEN];
        int color_code;
        int bytes_received = recv(client_socket, name, sizeof(name), 0);
        if (bytes_received <= 0)
            continue;
        recv(client_socket, &color_code, sizeof(color_code), 0);
        recv(client_socket, str, sizeof(str), 0);
            
        // Checking if the message is a system message or a user message
        bool isSystemMessage = (strcmp(name, "#NULL") == 0);
        
        // Decrypting user messages only
        if (!isSystemMessage) {
            for (int i = 0; str[i] != '\0'; ++i) {
                if (isalpha(str[i])) {
                    char base = islower(str[i]) ? 'a' : 'A';
                    str[i] = (str[i] - 3 - base + 26) % 26 + base;
                }
            }
        }
        
        // Displaying messages
        eraseText(6);
        if (!isSystemMessage)
            cout << color(color_code) << name << " : " << def_col << str << endl;
        else
            cout << color(color_code) << str << endl;
        cout << colors[1] << "You : " << def_col;
        fflush(stdout);
    }
}

// User signup function
bool signup() {
    string name, password;
    cout << "Enter name: ";
    cin >> name;
    cout << "Enter password: ";
    cin >> password;
    string hashedPassword = hashPassword(password);
    writeUserData(name, hashedPassword);
    return true; 
}

// User login function
bool login() {
    string password;
    char name[MAX_LEN];
    cout << "Enter your name : ";
    cin.getline(name, MAX_LEN);
    cout << "Enter password: ";
    cin >> password;
    string hashedPassword = hashPassword(password);
    string storedHash;
    if (readUserData(name, storedHash) && storedHash == hashedPassword) {
        send(client_socket, name, sizeof(name), 0);
        cin.ignore();
        cout << "Login successful!\n";
        cout << colors[NUM_COLORS - 1] << "\n\t  <><><>Welcome back " << name << "!<><><>" << endl << def_col;
        return true;
    } else {
        return false;
    }
}
