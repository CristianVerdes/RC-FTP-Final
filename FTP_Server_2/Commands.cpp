//
// Created by cristian on 14.01.2017.
//

#include <unistd.h>
#include <cstring>
#include <vector>
#include <sys/stat.h>
#include "Commands.h"
#include "Connection.h"
#include <dirent.h>
#include <algorithm>
#include <fcntl.h>
#include <sys/sendfile.h>

#include "createDB.h"

using namespace std;

// Constructor
Commands::Commands(Connection::thread_data threadData1) {
    threadData.client_descriptor = threadData1.client_descriptor;
    threadData.id_thread = threadData1.id_thread;
}

// Read and write functions
void Commands::writeToClient(string command) {
    printf("Writing to client: \"%s\" \n ", command.c_str());
    // Sends message to Client
    if (write(threadData.client_descriptor, command.c_str(), LENGTH) <= 0) {
        printf("[thread]-%d-\n", threadData.id_thread);
        perror("Failed to write() to client!\n");
    }
}

void Commands::readFromClientCommands() {
    // Read the clients command
    bzero(msg_from_client, LENGTH);
    if (read(threadData.client_descriptor, msg_from_client, sizeof(msg_from_client)) <= 0) {
        printf("[thread]-%d-", threadData.id_thread);
        perror("Failed to read()!\n");
    }

    // Here we start to proccess the command
    doCommand(string(msg_from_client));
}

// Database commands
string Commands::encryptDecrypt(string toEncrypt) {
    char key = 'K'; //Any char will work
    string output = toEncrypt;

    for (int i = 0; i < toEncrypt.size(); i++)
        output[i] = toEncrypt[i] ^ key;
    printf("Decrypted username and password: %s \n", output.c_str());
    return output;
}
void Commands::readUsernameAndPassword(){
    string decryptedUsernameAndPassword;
    // Read the clients username and password
    bzero(msg_from_client, LENGTH);
    if (read(threadData.client_descriptor, msg_from_client, sizeof(msg_from_client)) <= 0) {
        printf("[thread]-%d-", threadData.id_thread);
        perror("Failed to read()!\n");
    }
    // Decrypt username and password
    printf("Encrypted username and password: %s \n", msg_from_client);
    decryptedUsernameAndPassword = encryptDecrypt(string(msg_from_client));

    // Split username and password
    char* tokens;
    char* decrypedMsg = new char[decryptedUsernameAndPassword.length() + 1];
    strcpy(decrypedMsg, decryptedUsernameAndPassword.c_str());

    tokens = strtok(decrypedMsg," ");
    username = string (tokens);
    tokens = strtok(NULL," ");
    password = string(tokens);
    printf("username and password: %s, %s \n", username.c_str(), password.c_str());

}
bool Commands::findUserInDB(){
    readUsernameAndPassword();
    // Find if user and password match or exist
        // Open database
    int openDB;
    openDB = sqlite3_open("clients.db", &clients_db);
    if( openDB ){
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(clients_db));
        exit(0);
    } else {
        fprintf(stdout, "Opened database successfully\n");
    }
        // Search in database
    if (search_user(clients_db, username, password)) {
        printf("User FOUND in database.\n");
        return true;
    } else {
        printf("User NOT found in database.\n");
        return false;
    }
}
void Commands::insertClientInDatabase(){
    readUsernameAndPassword();
    // Insert username and password into the database
    insertData(clients_db, username, password);
    mkdirCommand(username.c_str());
}


// Commands
void Commands::doCommand(string commandAndParameter) {
    string command;
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
        // Only for copy commands
        if (command == "copy_f" || command == "copy_d" || command == "cat_f" || command == "cat_d"){
            tokens = strtok(NULL," ");
            copyDirectory = string(tokens);
        }
    } else {
        command = commandAndParameter;
    }

    // Find the command we want
    if(command == "quit"){
        quitCommand();
    } else if(command == "upload"){
        uploadCommand();
    } else if(command == "download"){
        downloadCommand();
    } else if(command == "mkdir"){
        mkdirCommand();
    } else if(command == "access"){
        accessCommand();
    } else if(command == "back"){
        backCommand();
    } else if(command == "delete_d"){
        deleteDIRCommand();
    } else if(command == "delete_f"){
        deleteFILECommand();
    } else if(command == "copy_d"){
        copyDIRCommand();
    } else if(command == "copy_f"){
        copyFILECommand();
    } else if(command == "list"){
        listCommand();
    } else if(command == "cat_f"){
        catFILECommand();
    } else if(command == "cat_d"){
        catDIRCommand();
    } else {
        wrongCommang();
    }

}

void Commands::quitCommand() {
    printf("Userul a apelat comanda |quit|\n");
    this->quit = true;
}

inline bool Commands::fileExists (string name) {
    if (FILE *file = fopen(name.c_str(), "r")) {
        fclose(file);
        return true;
    } else {
        return false;
    }
}

void Commands::uploadCommand() {
    char receiveBuffer[LENGTH];
    FILE *filePointer;
    bzero(receiveBuffer, LENGTH);
    // Create the destination of the folder
    string destinationOfTheFile = "/home/cristian/CLionProjects/NewFTP/FTP_Server_2/" +string(currentDirectory) + "/" + parameter;
    filePointer = fopen(destinationOfTheFile.c_str(), "a");
    if(filePointer == NULL){
        printf("File %s Cannot be opened or created on server.\n", parameter);
    } else {
        ssize_t fileBlockSize;
        while((fileBlockSize = read(threadData.client_descriptor, receiveBuffer, LENGTH)) > 0){
            ssize_t writeSize = fwrite(receiveBuffer, sizeof(char), fileBlockSize, filePointer);
            if(writeSize < fileBlockSize){
                printf("ERROR: File write failed on server. \n");
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
        printf("OK, received file from Client.\n");
        fclose(filePointer);
    }
    bzero(receiveBuffer, LENGTH);
    writeToClient("[server] upload");
}

void Commands::downloadCommand() {
    char sdbuf[LENGTH];
    bzero(sdbuf, LENGTH);
    //Deschidem fisierul
    string destinationOfTheFile = "/home/cristian/CLionProjects/NewFTP/FTP_Server_2/" +string(currentDirectory) + "/" + parameter;
    FILE *filePointer = fopen(destinationOfTheFile.c_str(), "r");
    if(filePointer == NULL){
        printf("ERROR: File %s not found.", parameter.c_str());
    }

    int fileBlockSize;
    while(fileBlockSize = fread(sdbuf, sizeof(char), LENGTH, filePointer )){
        if(write(threadData.client_descriptor, sdbuf, fileBlockSize) < 0){
            fprintf(stderr, "ERROR:: Failed to send file %s. (errno = %d)\n", parameter, errno);
            break;
        }
        bzero(sdbuf, LENGTH);
    }
    printf("[server] OK, file %s from Server was sent to Client.\n", parameter.c_str());
    usleep(2000);
    writeToClient("[server] download");
}

void Commands::mkdirCommand() {
    string directoryToCreate;
    directoryToCreate = string(currentDirectory) + "/"+ parameter;
    mkdirCommand(directoryToCreate.c_str());
    writeToClient("[server] directory " + parameter + " created");
}
void Commands::mkdirCommand(const char* username) {
    mkdir(username,S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
}

void Commands::accessCommand() {
    DIR *directory_pointer;

    string directoryToAccess;
    directoryToAccess = string(currentDirectory) + "/"+ parameter;

    directory_pointer = opendir(directoryToAccess.c_str());
    //Nu exista directorul specificat
    if(directory_pointer == NULL){
        string back_path="";
        string back_path2="";
        back_path = directoryToAccess;
        reverse(back_path.begin(),back_path.end());
        back_path2 = back_path.substr(back_path.find("/",0)+1,back_path.length());
        reverse(back_path2.begin(),back_path2.end());
        directoryToAccess = back_path2;
        printf("\n Back to: %s \n", directoryToAccess.c_str());
        writeToClient("[server] Could not open directory \n" + directoryToAccess + "> \n");
    } else {
        bzero(currentDirectory, 100);
        strcat(currentDirectory,directoryToAccess.c_str());
        writeToClient(directoryToAccess + "> \n");
    }
}

void Commands::backCommand() {
    string back_path="";
    string back_path2="";
    back_path = string(currentDirectory);
    reverse(back_path.begin(),back_path.end());
    back_path2 = back_path.substr(back_path.find("/",0)+1,back_path.length());
    reverse(back_path2.begin(),back_path2.end());
    bzero(currentDirectory, 100);
    strcpy(currentDirectory,back_path2.c_str());
    printf("\n Back to: %s \n", currentDirectory);

    writeToClient("[server] back to " + back_path2 + "> \n");
}

int Commands::delete_directory(const char*directory_Name){
    DIR *directory_pointer;
    struct dirent *directory_Content;

    char file_Path[FILENAME_MAX];
    string directoryPath;
    directoryPath = string(directory_Name);

    directory_pointer = opendir(directoryPath.c_str()); //deschidem directorul

    if(directory_pointer != NULL){
        //citim din director
        while(directory_Content = readdir(directory_pointer)){
            struct stat fileInfo; //structura pentru informatiile unui director/fisier din directorul principal

            snprintf(file_Path, FILENAME_MAX, "%s/%s", directoryPath.c_str(), directory_Content->d_name); //creem un path pentru fisierul/directorul citit

            //salvam in variabila fileInfo informatiile fisierului utilizand path-ul sau
            if(lstat(file_Path, &fileInfo )<0){
                perror(file_Path);
            }

            //vedem daca ce am citit este un director sau fisier
            if(S_ISDIR(fileInfo.st_mode)){
                if(strcmp(directory_Content->d_name, ".") && strcmp(directory_Content->d_name, "..")) {
                    printf("%s directory\n", file_Path);
                    delete_directory(file_Path);
                }
            }
            else {
                printf("%s file\n", file_Path);
                remove(file_Path);
            }
        }
        (void) closedir(directory_pointer);
    }
    else{
        perror ("Couldn't open the directory");
    }

    remove(directoryPath.c_str());
    return 0;
}
void Commands::deleteDIRCommand() {
    if (parameter != first_directory){
        string directoryPath;
        directoryPath = string(currentDirectory) +"/"+ parameter;
        delete_directory(directoryPath.c_str());
        writeToClient("[server] Directory deleted " + string(currentDirectory) + ">");
    } else {
        writeToClient("[server] Can't delete root file " + string(currentDirectory) + ">");
    }
}

void Commands::deleteFILECommand() {
    string filePath;
    filePath = string(currentDirectory) + "/" + parameter;
    remove(filePath.c_str());
    writeToClient("[server] delete file" + string(currentDirectory) + ">");

}

int Commands::copy_directory(const char* directory_Name, const char* new_directory){
    DIR *directory_pointer;
    DIR *directory_pointer2;
    mkdirCommand(new_directory);

    struct dirent *directory_Content;
    char file_Path[FILENAME_MAX];
    char file_Path2[FILENAME_MAX];


    directory_pointer = opendir(directory_Name); //deschidem directorul principal
    directory_pointer2 = opendir(new_directory); //deschidem directorul in care copiem


    if(directory_pointer != NULL){
        //citim din director
        while(directory_Content = readdir(directory_pointer)){
            struct stat fileInfo; //structura pentru informatiile unui director/fisier din directorul principal

            snprintf(file_Path, FILENAME_MAX, "%s/%s", directory_Name, directory_Content->d_name); //creem un path pentru fisierul/directorul citit
            snprintf(file_Path2, FILENAME_MAX, "%s/%s", new_directory, directory_Content->d_name); //creem un path pentru noul fisier

            //salvam in variabila fileInfo informatiile fisierului utilizand path-ul sau
            if(lstat(file_Path, &fileInfo )<0){
                perror(file_Path);
            }

            //vedem daca ce am citit este un director sau fisier
            if(S_ISDIR(fileInfo.st_mode)){
                if(strcmp(directory_Content->d_name, ".") && strcmp(directory_Content->d_name, "..")) {
                    printf("%s directory\n", file_Path);
                    mkdirCommand(file_Path2);
                    copy_directory(file_Path,file_Path2);
                }
            }
            else {
                printf("%s file\n", file_Path);
                copy_file(file_Path,file_Path2);
            }
        }
        (void) closedir(directory_pointer);
    }
    else{
        perror ("Couldn't open the directory");
    }
    return 0;
}
void Commands::copyDIRCommand() {
    copy_directory(parameter.c_str(), copyDirectory.c_str());
    writeToClient("[server] copy directory");
}

void Commands::copy_file(const char* original_file, const char* copied_file){
    int read_fileDescriptor;
    int write_fileDesctriptor;
    struct stat fileInfo;
    off_t offset = 0;

    //Deschidem fisierul
    read_fileDescriptor = open(original_file, O_RDONLY);
    //Preluam datele despre fisier
    fstat(read_fileDescriptor, &fileInfo);
    //Deschidem fisierul al doilea
    write_fileDesctriptor = open(copied_file,O_WRONLY | O_CREAT, fileInfo.st_mode);
    //Copiem fisierul
    sendfile(write_fileDesctriptor,read_fileDescriptor,&offset,fileInfo.st_size);
    //Inchidem fisierele
    close(read_fileDescriptor);
    close(write_fileDesctriptor);
}
void Commands::copyFILECommand() {
    //create file path o file to copy
    string filePath;
    filePath = string(currentDirectory) + "/" + parameter;
    copy_file(filePath.c_str(), copyDirectory.c_str());
    writeToClient("[server] copy file");
}

void Commands::catDIRCommand() {
    if (parameter != first_directory) {
        // Copy directory
        copy_directory(parameter.c_str(), copyDirectory.c_str());
        // Delete directory
        delete_directory(parameter.c_str());
        writeToClient("[server] cat directory " + string(currentDirectory) + ">");
    } else {
        writeToClient("[server] Can't cat the root directory " + string(currentDirectory) + ">");
    }


}

void Commands::catFILECommand() {
    // Copy File
        //create file path o file to copy
    string filePath;
    filePath = string(currentDirectory) + "/" + parameter;
    copy_file(filePath.c_str(), copyDirectory.c_str());

    // Delete File
    remove(filePath.c_str());
    writeToClient("[server] cat file " + string(currentDirectory) + ">");
}

string listing(const char* directory){
    DIR* directory_pointer;
    struct dirent *directory_content;
    string file_Path="\n[server]Listing directory ";
    file_Path+=directory;
    file_Path+=":\n";

    //Deschidem directorul
    directory_pointer = opendir(directory);

    if(directory_pointer != NULL){
        //Listam continutul

        while((directory_content = readdir(directory_pointer)) != NULL){
            if(!strcmp(directory_content->d_name,".") || !strcmp(directory_content->d_name,".."))
                continue;
            file_Path+=directory;
            file_Path+='/';
            file_Path+=directory_content->d_name;
            file_Path+="\n";
        }
        return file_Path;
    }
    else{
        return file_Path;
    }
}
void Commands::listCommand() {
    writeToClient(listing(currentDirectory));
}

void Commands::wrongCommang() {
    writeToClient("[server] Wrong command");
}

void Commands::bzeroTheBuffer() {
    bzero(this->msg_from_client, LENGTH);
}






