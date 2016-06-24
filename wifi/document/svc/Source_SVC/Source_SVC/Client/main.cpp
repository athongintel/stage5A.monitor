#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <chrono>

#include "SVCClient.hpp"
#include "AuthPKI.hpp"

#define BUFFER_SIZE 512

int main(int argc, const char * argv[]){
    auto start = std::chrono::high_resolution_clock::now();
    auto finish = std::chrono::high_resolution_clock::now();
    auto msChrono = std::chrono::duration<float, std::chrono::milliseconds::period>(finish-start);
    
    // Init the AuthInterface to use with our client identity
    SVCAuthInterface * auth = new AuthPKI("client");
    SVCClient client = SVCClient(auth);
    
    bool connected;
    
    if(argc<4){
        std::cout<<"Usage : client [server_address] [server_port] [path_save_file]";
		return 0;
    }
    const char * server_address = argv[1];
    const char * server_port = argv[2];
    const char * path = argv[3];
    
    try {
        
        /* Timer start (negociation) */
        std::cout << "Starting negociation timer" << std::endl;
        start = std::chrono::high_resolution_clock::now();
        
        client.connect(server_address, atoi(server_port));
        client.negociate();
        
        /* End timer (negociation) */
        finish = std::chrono::high_resolution_clock::now();
        msChrono = std::chrono::duration<float, std::chrono::milliseconds::period>(finish-start);
        std::cout << "Negociation time : " << msChrono.count() << "ms" << std::endl;
        
        connected = true;
    } catch (SVCException e) {
        std::cout<<e.what()<<std::endl;
        return 0;
    }
    
    std::cout<<"Connected to the server"<<std::endl;
    
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;
    
    FILE *fp = fopen(path,"wb");
    if(fp == NULL)throw std::runtime_error("Error opening file");
    size_t filelen;
    
    client.recv(&filelen, sizeof(size_t));
    std::cout<<"Receiving file of "<<filelen <<" octect"<<std::endl;
    bytes_read = 0;
    client.send(&bytes_read,sizeof(size_t));
    while(bytes_read<filelen){
        
        // Display progress
        std::cout << "\r" << ((float)bytes_read / (float)filelen)*100 << "%           " ;
        
        // Reconnect if needed
        while(!connected){
            try{
                /* Timer start (negociation) */
                std::cout << "Starting negociation timer" << std::endl;
                start = std::chrono::high_resolution_clock::now();
                
                client.connect(server_address, atoi(server_port));
                client.negociate();
                
                /* End timer (negociation) */
                finish = std::chrono::high_resolution_clock::now();
                msChrono = std::chrono::duration<float, std::chrono::milliseconds::period>(finish-start);
                std::cout << "Negociation time : " << msChrono.count() << "ms" << std::endl;
                
                connected = true;
                
                // Get filesize and ask to start where we stoped
                client.recv(&filelen,sizeof(size_t));
                client.send(&bytes_read, sizeof(size_t));
            }
            catch(SVCException e){
                std::cout<<e.what()<<std::endl;
                std::cout<<"Impossible to connect, trying to reconnect in 1 second.."<<std::endl;
                sleep(1);
            }
        }
        
        bzero(buffer, BUFFER_SIZE);
        
        if(bytes_read + BUFFER_SIZE >filelen){
            try {
                client.recv(buffer, filelen-bytes_read);
                fwrite(buffer, sizeof(char), filelen-bytes_read, fp);
                bytes_read += filelen-bytes_read;
                
            } catch (SVCException e) {
                std::cout<<e.what()<<std::endl;
                if(e.getError() == ERR_DATA_RECV){
                    connected = false;
                    std::cout<<"Connection Lost, trying to reconnect.."<<std::endl;
                }
            }
        }
        else{
            try {
                client.recv(buffer, BUFFER_SIZE);
                fwrite(buffer, sizeof(char), BUFFER_SIZE, fp);
                bytes_read +=BUFFER_SIZE;
                
            } catch (SVCException e) {
                std::cout<<e.what()<<std::endl;
                if(e.getError() == ERR_DATA_RECV){
                    connected = false;
                    std::cout<<"Connection Lost, trying to reconnect.."<<std::endl;
                }
            }
        }
    }
}