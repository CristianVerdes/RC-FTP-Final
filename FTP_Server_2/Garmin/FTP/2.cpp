#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include<string>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <sys/wait.h>
#include <iostream>
#include <string>
using namespace std;

#define LENGTH 512

int PORT;
int sd;            // descriptorul de socket
char command_from_console[LENGTH];
char msg_from_server[LENGTH];
char msg_to_server[LENGTH];
struct sockaddr_in server;    // structura folosita pentru conectare
int fd;
struct stat file_stat;
char file_size[256];
ssize_t len;
int uploadIsDone = 0;

//upload and download
char receiveBuffer[LENGTH];

void uploadCommand(int serverDescriptor, const char* fileName) {
    char sdbuf[LENGTH];
    bzero(sdbuf, LENGTH);
    //Deschidem fisierul
    FILE *filePointer = fopen(fileName, "r");
    if(filePointer == NULL){
        printf("ERROR: File %s not found.", fileName);
    }

    int fileBlockSize;
    while(fileBlockSize = fread(sdbuf, sizeof(char), LENGTH, filePointer )){
        if(write(sd, sdbuf, fileBlockSize) < 0){
            fprintf(stderr, "ERROR:: Failed to send file %s. (errno = %d)\n", fileName, errno);
            break;
        }
        bzero(sdbuf, LENGTH);
    }
    bzero(sdbuf, LENGTH);
    usleep(2000);
    printf("[client] OK, file %s from Client was sent to Server.\n", fileName);
}

void downloadCommand(int serverDescriptor, const char* fileName) {
    bzero(receiveBuffer, LENGTH);
    // Create the destination of the folder
    string destinationOfTheFile =  string(fileName);
    FILE *filePointer = fopen(destinationOfTheFile.c_str(), "a");
    if(filePointer == NULL){
        printf("File %s Cannot be opened or created on server.\n", destinationOfTheFile.c_str());
    } else {
        ssize_t fileBlockSize;
        while((fileBlockSize = read(serverDescriptor, receiveBuffer, LENGTH)) > 0){
            ssize_t writeSize = fwrite(receiveBuffer, sizeof(char), fileBlockSize, filePointer);
            if(writeSize < fileBlockSize){
                printf("ERROR: File write failed on client. \n");
            }
            bzero(receiveBuffer, LENGTH);
            if(fileBlockSize == 0 || fileBlockSize != 512){
                break;
            }
        }
        if(fileBlockSize < 0){
            if (errno == EAGAIN){
                printf("recv() timed out.\n");
            } else {
                fprintf(stderr, "recv() failed due to errno = %d \n", errno);
                exit(1);
            }
        }
        printf("OK, received file from Server.\n");
        printf("Closing file\n");
        if(fclose(filePointer)!=0){
            printf("Failed to close file pointer due to errno = %d \n", errno);
        }
        printf("File closed\n");
    }
    printf("Start bzero \n");
    bzero(receiveBuffer, LENGTH);
    printf("End bzero \n");
}

void uploadOrDownload(string commandAndParameter) {
    string command;
    string parameter;
    // Verify type of command
    if (commandAndParameter.find(" ") != std::string::npos) {
        // Split into command and parameter
        char* tokens;
        char* decrypedMsg = new char[commandAndParameter.length() + 1];
        strcpy(decrypedMsg, commandAndParameter.c_str());

        tokens = strtok(decrypedMsg," ");
        command = string (tokens);
        tokens = strtok(NULL," ");
        parameter = string(tokens);
    } else {
        command = commandAndParameter;
    }

    if(command == "upload") {
        uploadCommand(sd, parameter.c_str());
    }
    if(command == "download") {
        downloadCommand(sd, parameter.c_str());
    }
}

int commandsAfterConnection(){
    // Read from server
    if (read(sd, msg_from_server, LENGTH) < 0) {
        perror("[client]Eroare la read() de la server.\n");
        return errno;
    }

    // Showing what we received
    printf("%s\n", msg_from_server);

    while (1) {
        // Read what the client is typing
        bzero((void *) command_from_console, LENGTH);
        read(0, (void *) command_from_console, sizeof(command_from_console));
        printf("[client] Am citit %s", command_from_console);
        // Remove '\n'
        strncpy(msg_to_server, command_from_console, strlen(command_from_console) - 1);

        // Send to server the command
        printf("Writing to server: \"%s\" \n", msg_to_server);
        if (write(sd, (void *) msg_to_server, sizeof(msg_to_server)) <= 0) {
            perror("[client]Eroare la write() spre server. (Upload)\n");
            return errno;
        }
        // EXIT APP
        if (strcmp(msg_to_server, "quit") == 0){
            return 0;
        }
        printf("Command started \n");
        // Only for Upload and download
        uploadOrDownload(string(msg_to_server));
        printf("Command ended \n");
        // Read from server what the command did
        printf("Waiting to read...");
        if (read(sd, msg_from_server, LENGTH) < 0) {
            perror("[client]Eroare la read() de la server.\n");
            return errno;
        }

        // Showing what we received
        printf("%s\n", msg_from_server);

        // Bzero the buffer
        bzero(msg_from_server, LENGTH);
        bzero(msg_to_server, LENGTH);
        bzero(command_from_console, LENGTH);
    }
}

string encryptDecrypt(string toEncrypt) {
    char key = 'K'; //Any char will work
    string output = toEncrypt;

    for (int i = 0; i < toEncrypt.size(); i++)
        output[i] = toEncrypt[i] ^ key;
    printf("Encrypted username and password: %s \n", output.c_str());
    return output;
}

int sendUsernameAndPassword(){
    printf("Enter username and password separated by space: \n");

    // Read what the client is typing
    bzero((void *) command_from_console, LENGTH);
    read(0, (void *) command_from_console, sizeof(command_from_console));
    printf("[client] Am citit %s", command_from_console);

    // Encrypt username and password
    string encryptedMsg = encryptDecrypt(string(command_from_console));
    // Remove '\n'
    strncpy(msg_to_server, encryptedMsg.c_str(), strlen(encryptedMsg.c_str()) - 1);
    // Send to server encrypted username and password
    if (write(sd, (void *) msg_to_server, sizeof(msg_to_server)) <= 0) {
        perror("[client]Eroare la write() spre server. (Upload)\n");
        return errno;
    }


    // Bzero the buffer
    bzero(msg_to_server, LENGTH);
    bzero(command_from_console, LENGTH);
    return 0;
}
bool verifyLoginAnwser(){
    // Verify login answer
        // Read from server
    if (read(sd, msg_from_server, LENGTH) < 0) {
        perror("[client]Eroare la read() de la server.\n");
        return errno;
    }
    if(!strcmp(msg_from_server,"[server] Failed to login")){
        // Bzero last buffer
        printf(" %s \n ", msg_from_server);
        bzero(msg_from_server, LENGTH);
        return false;
    } else {
        // Bzero last buffer
        bzero(msg_from_server, LENGTH);
        return true;
    }

}

int main (int argc, char *argv[]) {
    /* exista toate argumentele in linia de comanda? */
    if (argc != 3) {
        printf("Sintaxa: %s <adresa_server> <port>\n", argv[0]);
        return -1;
    }

    /* stabilim portul */
    PORT = atoi(argv[2]);

    /* cream socketul */
    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Eroare la socket().\n");
        return errno;
    }

    /* umplem structura folosita pentru realizarea conexiunii cu serverul */
    /* familia socket-ului */
    server.sin_family = AF_INET;
    /* adresa IP a serverului */
    server.sin_addr.s_addr = inet_addr(argv[1]);
    /* portul de conectare */
    server.sin_port = htons(PORT);

    /* ne conectam la server */
    if (connect(sd, (struct sockaddr *) &server, sizeof(struct sockaddr)) == -1) {
        perror("[client]Eroare la connect().\n");
        return errno;
    }

    /*/////////////////////////////////////////////// START THE FUN!!! \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

    // Connect to the server
    sendUsernameAndPassword();
    bool connected = verifyLoginAnwser();

    // If login failed
    if(!connected){
        // Create a new user and password
        sendUsernameAndPassword();
        // Read from server login status;
        if (read(sd, msg_from_server, LENGTH) < 0) {
            perror("[client]Eroare la read() de la server.\n");
            return errno;
        }
        printf("%s \n", msg_from_server);
        bzero(msg_from_server, LENGTH);
    }

    // Do comamnds after connection
    commandsAfterConnection();

    return 0;
}















































