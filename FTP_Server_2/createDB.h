//
// Created by cristian on 17.01.2017.
//

#ifndef FTP_SERVER_2_CREATEDB_H
#define FTP_SERVER_2_CREATEDB_H


#include <sqlite3.h>
#include <iostream>
#include <string>
using namespace std;

static int callback(void *data, int argc, char **argv, char **azColName);
void insertData(sqlite3 *clients_db, string username, string password);
string  encryptDecrypt(string toEncrypt);
int search_user(sqlite3 *clients_db, string password, string username);




#endif //FTP_SERVER_2_CREATEDB_H
