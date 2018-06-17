//
// Created by cristian on 14.01.2017.
//

#ifndef FTP_SERVER_2_CONNECTION_H
#define FTP_SERVER_2_CONNECTION_H


#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h> // For sockaddr_in
#include <pthread.h>
#include <iostream>

class Connection {
public:
    // Structure for thread data
    typedef struct thread_data{
        int id_thread; //the id of the thread
        int client_descriptor; //the descriport returned by accept
    }thread_data;

    // Threads declaration
    int threads_counter = 0;
    pthread_t threads[10000];

    // Server Socket declaration
    int socket_descriptor;
    int client_socket;
    socklen_t client_length;
    struct sockaddr_in server;
    struct sockaddr_in client;

    void createConnection(int PORT);
};


#endif //FTP_SERVER_2_CONNECTION_H
