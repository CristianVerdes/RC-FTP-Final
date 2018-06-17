//
// Created by cristian on 14.01.2017.
//

#include <strings.h>
#include <pthread.h>
#include <cstring>
#include "Connection.h"
#include "Commands.h"
#include <unistd.h>

void *treatClient(void *arg);
void raspunde(void *arg);
void Connection::createConnection(int PORT) {
    //Create socket

    if((socket_descriptor = socket(AF_INET, SOCK_STREAM,0))== -1) {
        perror("Error at creating the socket");
    }
    printf("Socket created...%d\n", socket_descriptor);

    //Using th option SO_REUSEADDR
    int on=1;
    setsockopt(socket_descriptor,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on));

    //Preparing the data structures
    bzero (&server, sizeof (server));
    bzero (&client, sizeof (client));

    //We prepare the sockaddr_in structure
    server.sin_family = AF_INET; //Family of sockets
    server.sin_addr.s_addr = INADDR_ANY; //Acept any address
    server.sin_port = htons (PORT); //PORT

    //Bind
    if(bind(socket_descriptor, (struct sockaddr *) &server, sizeof(server)) < 0){
        perror("Bind failed!");
        return;
    }
    printf("Bind done.\n");

    //Listen for clients
    if(listen(socket_descriptor,2)<0){
        perror("Listen failed!");
        return;
    }


    //Serve the clients using threads
    while(1){
        int  client;
        client_length = sizeof(client);

        printf("[Server] Waiting at port %d...\n", PORT);

        if((client = accept (socket_descriptor, (struct sockaddr *) &client, &client_length )) <0){
            perror("Accept failed!");
            return;
        }

        thread_data * thread_data_pointer;
        thread_data_pointer = (struct Connection::thread_data*) malloc(sizeof(struct Connection::thread_data));
        thread_data_pointer->id_thread = threads_counter++;
        thread_data_pointer->client_descriptor = client;

        printf("Client descriptor: %d\n",thread_data_pointer->client_descriptor);
        pthread_create(&threads[threads_counter], NULL, &treatClient, thread_data_pointer);
    }
}
void *treatClient(void *arg) {
    struct Connection::thread_data threadData;
    threadData= *((Connection::thread_data*) arg);
    printf ("[thread]- %d -Waiting for the message...\n", threadData.id_thread);
    pthread_detach(pthread_self());
    // Raspundem clientului
    raspunde((struct thread_data*) arg);
    printf("\nEnd of client %d\n", threadData.id_thread);
    // Closing the connection
    close(threadData.client_descriptor);
    return NULL;
}

void clientCommandsAfterLogin(Commands commands){
    commands.bzeroTheBuffer();
    commands.writeToClient("Welcome to the FTP server! Enter your command... \n " + commands.username + "> ");
    // Set root of files
    strcat(commands.first_directory,commands.username.c_str());
    // Set current accessed folder
    strcat(commands.currentDirectory,commands.username.c_str());
    // Do the commands
    while(!commands.quit){
        //Read command from client
        commands.readFromClientCommands();

        // Bzero
        commands.bzeroTheBuffer();
    }
}

void raspunde(void *arg) {
    struct Connection::thread_data threadData;
    threadData= *((Connection::thread_data*) arg);

    Commands commands(threadData);


    // Find user in db or add a new one
    if(commands.findUserInDB()){
        commands.writeToClient("[server] Login successful!");
        clientCommandsAfterLogin(commands);
        return;
    } else {
        commands.writeToClient("[server] Failed to login");
        // create a new user and insert it to the database
        commands.insertClientInDatabase();
        commands.writeToClient("[server] Created new user");
        clientCommandsAfterLogin(commands);
    }


}




































