#include <chrono>
#include <grpcpp/grpcpp.h>
#include <grpcpp/security/server_credentials.h>
#include <iostream>
#include "server.h"
#include <sstream>
#include <thread>
#include "configparser.h"

int main(int,char**)
{
    Server server;
    grpc::ServerBuilder b;
    ConfigJsonReader parser;
    bool stat= parser.LoadFile("param.json");
    std::stringstream log;
    if(stat){
        log << parser.host;
        log <<":";
        log << parser.port;
        server.m_com_servis = parser.com;
        server.m_bautrate_servis = parser.bautrate;
        std::this_thread::sleep_for(std::chrono::seconds(6));
        printf("\nrunning openpinpad first..");
        server.RunOpenPinpad(parser.host_pinpad, parser.port_pinpad);
    }else{ 
        int port = 9889;
        log <<"127.0.0.1:";
        log <<port;
    }
    b.AddListeningPort(log.str(), grpc::InsecureServerCredentials());
    b.RegisterService(&server);
    std::unique_ptr<grpc::Server> serv(b.BuildAndStart());
    std::cout <<"\nlisten servis cardreader on "<<log.str();
    serv->Wait();
    return 0;
}