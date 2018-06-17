//
// Created by cristian on 14.01.2017.
//

#ifndef FTP_SERVER_2_COMMANDS_H
#define FTP_SERVER_2_COMMANDS_H

#include <string>
#include <iostream>
#include "Connection.h"

#include <sqlite3.h>

#define LENGTH 512

using namespace std;

class Commands {
public:
    // Structure for thread data
    typedef struct thread_data{
        int id_thread; //the id of the thread
        int client_descriptor; //the descriport returned by accept
    }thread_data;

    thread_data threadData;
    bool quit = false;
    char msg_from_client[LENGTH];
    char standard_msg_response[LENGTH]="\n[server] Comenzi: \n1) Utilizati comanda |connect| pentru a va conecta la srv. \n2) Utilizati comanda |quit| pentru a inchide aplicatia. ";

    // Constructor
    Commands(Connection::thread_data threadData);

    // Read and write functions
    void writeToClient(string command);
    void bzeroTheBuffer();
    void readFromClientCommands();

    // Connect to database
    string username;
    string password;
    sqlite3 *clients_db;
    string encryptDecrypt(string toEncrypt);
    void readUsernameAndPassword();
    bool findUserInDB();
    void insertClientInDatabase();

    // Commands
    char first_directory[200];
    char currentDirectory[100];
    string parameter;
    string copyDirectory;
    int counter=0;
    void doCommand(string command);

    void quitCommand();

    inline bool fileExists (string name);
    void uploadCommand();

    void downloadCommand();

    void mkdirCommand();

    void mkdirCommand(const char* username);

    void accessCommand();

    void backCommand();

    int delete_directory(const char*directory_Name);
    void deleteDIRCommand();

    void deleteFILECommand();

    int copy_directory(const char* directory_Name, const char* new_directory);
    void copyDIRCommand();

    void copy_file(const char* original_file, const char* copied_file);
    void copyFILECommand();

    void listCommand();

    void wrongCommang();

    void catFILECommand();

    void catDIRCommand();
};


#endif //FTP_SERVER_2_COMMANDS_H
