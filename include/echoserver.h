#pragma once
#include "baseserver.h"

class EchoServer : public BaseServer {
public:

    EchoServer();

    ~EchoServer();

    virtual int ParseConfig(int argc, char** argv) override;


private:
    virtual void Worker(void* param) override;

};