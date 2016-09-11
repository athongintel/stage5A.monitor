#include <iostream>
#include <signal.h>
#include <stdlib.h>
#include <iomanip>
#include <stdio.h>
#include <thread>
#include <mutex>

#include "TCPServerSocket.hpp"
#include "SVCServer.hpp"
#include "SVCServer.hpp"
#include "AuthPKI.hpp"


#define BUFFER_SIZE 512

std::mutex fileMtx;
SVCServer *pserver = NULL;

auto start = std::chrono::high_resolution_clock::now();
auto finish = std::chrono::high_resolution_clock::now();
auto msChrono = std::chrono::duration<float, std::chrono::milliseconds::period>(finish-start);

static void s_signal_handler (int signal_value)
{
    if(pserver !=NULL)pserver->close();
}

void handleclient(SVCConnectedClient * client,std::string * path){
    ssize_t bytes_read,bytes_send,offset;
    unsigned char buffer[BUFFER_SIZE];
    FILE * fp;
    
    try{
        /* Timer start (negociation) */
        std::cout << "Starting negociation timer" << std::endl;
        start = std::chrono::high_resolution_clock::now();
        
        // Don't display text while benchmarking negociation tme
        //std::cout<<"Client connected from : "<<client->getForeignAddress()<<":"<<client->getForeignPort()<<std::endl;
        
        client->negociate();
        /* End timer (negociation) */
        finish = std::chrono::high_resolution_clock::now();
        msChrono = std::chrono::duration<float, std::chrono::milliseconds::period>(finish-start);
        std::cout << "Negociation time : " << msChrono.count() << "ms" << std::endl;
        
        // Open the file to send
        fp = fopen(path->c_str(),"rb");
        if(fp == NULL)throw std::runtime_error("Error opening file");
        
        fseek(fp, 0, SEEK_END);
        size_t fileLen = ftell(fp);
        rewind(fp);
        
        client->send(&fileLen, sizeof(size_t)); // Send file size
        client->recv(&offset,sizeof(size_t));   // Gets start offset
        
        fseek(fp, offset, SEEK_SET);
        
        bytes_send = 0;
        while (bytes_send+offset < fileLen){
            fileMtx.lock();
            bytes_read = fread(buffer,sizeof(char),BUFFER_SIZE,fp);
            fileMtx.unlock();
            if(bytes_read == 0)break;
            if(bytes_read<0)throw std::runtime_error("Error reading file");
            if(bytes_send > 1000000000)throw SVCException("ters");
            client->send(buffer, bytes_read);
            bytes_send += bytes_read;
        }
        
        std::cout<<"File sent"<<std::endl;
        client->close();
        delete client;
        
    }catch(SVCException e){
        fclose(fp);
        client->close();
        std::cout<<e.what()<<std::endl;
    }
}

int main(int argc, const char * argv[]){
    SVCAuthInterface * auth = new AuthPKI("server");
    
    if(argc<3){
        std::cout<<"Usage Server [port] [path_to_file]"<<std::endl;
        return 0;
    }
    int port = atoi(argv[1]);
    std::string * path = new std::string(argv[2]);
    std::cout<<"Sending file " << path->c_str()<<std::endl;
    struct sigaction action;
    action.sa_handler = s_signal_handler;
    action.sa_flags = 0;
    sigemptyset (&action.sa_mask);
    sigaction (SIGINT, &action, NULL);
    sigaction (SIGTERM, &action, NULL);
    
    try {
        SVCServer server = SVCServer(auth);
        pserver = &server;
        server.start(port);

        //Handle SIGPIPE SIGNAL
        sigset_t sigpipe_mask;
        sigemptyset(&sigpipe_mask);
        sigaddset(&sigpipe_mask, SIGPIPE);
        sigset_t saved_mask;
        
        if (pthread_sigmask(SIG_BLOCK, &sigpipe_mask, &saved_mask) == -1) {
            perror("pthread_sigmask");
            exit(1);
        }
        
        while(true){
            try{
                SVCConnectedClient * client = server.accept();
                new std::thread(handleclient,client,path);
                
            }catch(SVCException e){
                std::cout<<e.what()<<std::endl;
            }
        }
        
    } catch (SVCException e) {
        std::cout<<e.what()<<std::endl;
    }
    
    delete auth;
}
