#include <iostream>
#include <cstring>
#include <cerrno>

#include "baseserver.h"
#include "cxxopts.hpp"

using namespace std;

BaseServer::BaseServer() {
    vflag_ = 0;
    fd_cnt_ = 0;
}

BaseServer::~BaseServer() {}

bool BaseServer::SetupListenFd() {
    listen_fd_ = socket(PF_INET, SOCK_STREAM, 0);
    cout << "Created listen fd " << listen_fd_ << endl;
    if(listen_fd_ == -1) {
        fprintf(stderr, "Failed to open a listen socket.\n");
        return false;
    }
    cout << "Opened listen fd." << endl;

    return true;
}

bool BaseServer::SetupListenAddr() {
    server_inet_.sin_family = AF_INET;
    server_inet_.sin_addr.s_addr = htons(INADDR_ANY);
    server_inet_.sin_port = htons(4711);

    if(bind(listen_fd_, (sockaddr*)&server_inet_, sizeof(server_inet_)) < 0) {
        cout << "Failed to bind addr";
        return false;
    }

    cout << "Successfully bind on 0.0.0.0:4711" << endl;
    return true;
}

int BaseServer::Run() {
    SetupListenFd();
    SetupListenAddr();
    listen(listen_fd_, 10);

    while(true) {
        sockaddr client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        int comm_fd = accept(listen_fd_, (sockaddr*) &client_addr, &client_addr_len);
        if(comm_fd == -1) {
            cout << "Failed to accept" << endl;
            cout << "Error: " << strerror(errno) << endl;
            return 1;
        }
        cout << "New client connects on " << comm_fd << endl;
        AddFd(comm_fd);

        thread new_thread([&] {Worker(&comm_fd); });
        new_thread.detach();
    }

    return 0;
}

int BaseServer::Stop() {
    return 0;
}

bool BaseServer::AddFd(int fd) {
    if(fd_cnt_ == MAX_CLIENTS) {
        return false;
    }
    fd_list_mutex_.lock();
    active_fd_[fd_cnt_] = fd;
    fd_cnt_++;
    fd_list_mutex_.unlock();
    return true;
}

bool BaseServer::RemoveFd(int fd) {
    //1.get index of that fd
    // if doesn't exist, return false
    int idx = -1;
    for(int i = 0; i < fd_cnt_; i++) {
        if(active_fd_[i] == fd) {
        idx = i;
        break;
        }
    }  

    if(idx == -1) {
        cout << "File Descriptor " << fd << " does not exist" << endl;
        return false;
    }

    //2.remove it, and decrease fd_cnt
    fd_list_mutex_.lock();
    for(int i = idx; i + 1 < MAX_CLIENTS; i++) {
        active_fd_[i] = active_fd_[i + 1];
    }
    fd_cnt_--;
    //client_cnt_--;
    fd_list_mutex_.unlock();
 
    return true;
}

bool BaseServer::SendToSocket(int fd, string text) {
    int write_len = text.size();
    const char* c_text = text.c_str();
    DoWrite(fd, c_text, write_len);
    return true;
}

bool BaseServer::SendErrResponse(int fd, string text) {
    SendToSocket(fd, "-ERR " + text);
    return true;
}

bool BaseServer::SendOkResponse(int fd, string text) {
    SendToSocket(fd, "+OK " + text);
    return true;
}

bool BaseServer::DoRead(int conn_fd, char buffer[]) {
    char tmp;
    int i = 0;

    for(; i < BUFF_SIZE; i++) {
        bool got_cr = false;
        int flag = read(conn_fd, &tmp, 1);
        if(flag < 0) {
            fprintf(stderr, "Read failed.\n");
            return false;
        }
        
        if(flag == 0) {
            printf("Read finished.\n");
            break;
        }

        if(tmp == '\n') {
            break;
        }
        if(tmp == '\r') {
            got_cr = true;
            continue;
        }
        if(got_cr) {
            buffer[i] = '\r';
            i++;
        }
        
        buffer[i] = tmp;
    }

    buffer[i + 1] = '\0';

    return true;
}

bool BaseServer::DoWrite(int conn_fd, const char* buffer, int len) {
    int sent = 0;
    while(sent < len) {
        int n = write(conn_fd, &buffer[sent], len - sent);
        if(n < 0) {
            fprintf(stderr, "Failed to send a message from server to client\n");
            return false;
        }
        sent += n;
    }
    return true;
}