#include <iostream>

#include "cxxopts.hpp"
#include "echoserver.h"

using namespace std;

EchoServer::EchoServer() : BaseServer() {}

EchoServer::~EchoServer() { }

int EchoServer::ParseConfig(int argc, char** argv) {
    return 0;
}

//Default address: 0.0.0.0:4711
void EchoServer::Worker(void* param) {
    int comm_fd = *(int*)param;
    cout << "Echo worker starts" << endl;

    string greeting_message = "+OK Server ready (Author: Haochen Gao /hcgao)\r\n";
    DoWrite(comm_fd, greeting_message.c_str(), greeting_message.size());
    
    while(true) {
        char buffer[BUFF_SIZE];
        memset(buffer, 0, sizeof(char) * BUFF_SIZE);
        DoRead(comm_fd, buffer);
        cout << "Received [" << buffer << "] from" << comm_fd << endl; 

        string readin_string(buffer);
        string command = readin_string.size() >= 4 ? readin_string.substr(0, 4) : "UNKNOWN";
        string message = readin_string.size() >= 6 ?readin_string.substr(5, readin_string.size() - 5) : "";
        message.append("\r\n");

        if(command == "ECHO") {
            string echo_str = "+OK ";
            DoWrite(comm_fd, echo_str.c_str(), echo_str.size());
            const char* c_msg = message.c_str();
            DoWrite(comm_fd, c_msg, message.size());
            cout << "Sent [" << c_msg << "] to fd " << comm_fd << endl;

        } else if(command == "QUIT") {
            string quit_str = "+OK Goodbye!\r\n";
            DoWrite(comm_fd, quit_str.c_str(), quit_str.size());
            cout << "Connection " << comm_fd << " closed" << endl;
            break;
        } else {

            string unknown_str = "-ERR Unknown command\r\n";
            DoWrite(comm_fd, unknown_str.c_str(), unknown_str.size());
            cout << "Delt with unkown message" << endl;
        }
    }

    RemoveFd(comm_fd);
    close(comm_fd);
}