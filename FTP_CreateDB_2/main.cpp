#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <string>
#include <iostream>
#include <string.h>
#include "sqlite/sqlite3.h"
using namespace std;

char ** argv_count;
int count = 0;
static int callback(void *data, int argc, char **argv, char **azColName){
    count = atoi(argv[0]);
    printf("\n");
    return 0;
}
void createTable(sqlite3 *clients_db){
    char *sql;
    int  rc;
    char *errMsg = 0;

    // Create SQL statement
    sql = "CREATE TABLE CLIENTS("  \
         "USERNAME       TEXT    NULL," \
         "PASSWORD       TEXT    NULL," \
         "LOGIN_BAN      INT NULL," \
         "UPLOAD_BAN     INT NULL," \
         "DOWNLOAD_BAN   INT NULL," \
         "MKDIR_BAN      INT NULL," \
         "COPYDIR_BAN    INT NULL);";

    /* Execute SQL statement */
    rc = sqlite3_exec(clients_db, sql, callback, 0, &errMsg);
    if( rc != SQLITE_OK ){
        fprintf(stderr, "SQL error: %s\n", errMsg);
        sqlite3_free(errMsg);
    }else{
        fprintf(stdout, "Table created successfully\n");
    }
}
int selectUsername(sqlite3 *clients_db, char username[100]){
    string sql;
    int  rc;
    char *errMsg = 0;

    //Select * from table
    sql = "SELECT PASSWORD FROM CLIENTS WHERE USERNAME='";
    sql += username ;
    sql += "';";

    const char* data = "Callback function called";

    /* Execute SQL statement */
    const char *cstr = sql.c_str();
    rc = sqlite3_exec(clients_db, cstr, callback, (void*)data, &errMsg);
    if( rc != SQLITE_OK ){
        fprintf(stderr, "SQL error: %s\n", errMsg);
        sqlite3_free(errMsg);
        return 1;
    }else {
        fprintf(stdout, "Operation done successfully\n");
        return 0;
    }
}
void deleteTable(sqlite3 *clients_db){
    char *sql;
    int  rc;
    char *errMsg = 0;

    //Delete Table
    sql = "DROP TABLE CLIENTS;";

    /* Execute SQL statement */
    rc = sqlite3_exec(clients_db, sql, callback, 0, &errMsg);
    if( rc != SQLITE_OK ){
        fprintf(stderr, "SQL error: %s\n", errMsg);
        sqlite3_free(errMsg);
    }else{
        fprintf(stdout, "Table deleted successfully\n");
    }
}
void selectTable(sqlite3 *clients_db){
    char *sql;
    int  rc;
    char *errMsg = 0;

    //Select * from table
    sql = "SELECT * FROM CLIENTS";
    const char* data = "Callback function called";

    /* Execute SQL statement */
    rc = sqlite3_exec(clients_db, sql, callback, (void*)data, &errMsg);
    if( rc != SQLITE_OK ){
        fprintf(stderr, "SQL error: %s\n", errMsg);
        sqlite3_free(errMsg);
    }else {
        fprintf(stdout, "Operation done successfully\n");
    }
}
void insertData(sqlite3 *clients_db, string username, string password){
    string sql;
    int  rc;
    char *errMsg = 0;

    //Insert into table
    sql = "INSERT INTO CLIENTS (USERNAME,PASSWORD) "  \
         "VALUES ('mircea.cretu', '12345asd'); " \
         "INSERT INTO CLIENTS (USERNAME,PASSWORD) "  \
         "VALUES ('ion', '123abc'); " \
         "INSERT INTO CLIENTS (USERNAME,PASSWORD) "  \
         "VALUES ('mihai_blegu', '12345'); " \
         "INSERT INTO CLIENTS (USERNAME,PASSWORD) "  \
         "VALUES ('eu', '12345'); ";

    //Insert into table
//    sql = "INSERT INTO CLIENTS (USERNAME,PASSWORD) "  \
//         "VALUES ('";
//    sql += username ;
//    sql += "', '";
//    sql += password;
//    sql += "'); ";
//    cout << "SQL: " <<sql <<'\n';
    const char *cstr = sql.c_str();

    // Execute SQL statement
    rc = sqlite3_exec(clients_db, cstr, callback, 0, &errMsg);
    if( rc != SQLITE_OK ){
        fprintf(stderr, "SQL error: %s\n", errMsg);
        sqlite3_free(errMsg);
    }else{
        fprintf(stdout, "Records created successfully\n");
    }
};
int search_user(sqlite3 *clients_db, string password, string username){
    string sql;
    int  rc;
    char *errMsg = 0;

    //Select * from table
    sql = "SELECT count(*) FROM CLIENTS WHERE USERNAME LIKE '";
    sql += username ;
    sql += "' AND PASSWORD LIKE '";
    sql += password;
    sql += "';";
    const char* data = "Callback function called";
    printf("\n...%s\n",sql.c_str());

    /* Execute SQL statement */
    rc = sqlite3_exec(clients_db, sql.c_str(), callback, (void*)data, &errMsg);
    if( rc != SQLITE_OK ){
        fprintf(stderr, "SQL error: %s\n", errMsg);
        sqlite3_free(errMsg);

    }else {
        fprintf(stdout, "Operation done successfully\n");
        printf("%d\n", count);
    }


}

string encryptDecrypt(string toEncrypt) {
    char key = 'K'; //Any char will work
    string output = toEncrypt;

    for (int i = 0; i < toEncrypt.size(); i++)
        output[i] = toEncrypt[i] ^ key;

    return output;
}

int main(int argc, char* argv[])
{
    sqlite3 *clients_db;
    int  rc;
    string username="mircea";
    string password="1234";
    password = encryptDecrypt(password);
    cout << "Encrypted:" << password << "\n";

    string decrypted = encryptDecrypt(password);
    cout << "Decrypted:" << decrypted << "\n";

    //Open database
    rc = sqlite3_open("clients.db", &clients_db);
    if( rc ){
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(clients_db));
        exit(0);
    }else{
        fprintf(stdout, "Opened database successfully\n");
    }

    //createTable(clients_db);
    //deleteTable(clients_db);
    //insertData(clients_db, username, decrypted);
    selectTable(clients_db);
    //selectUsername(clients_db,"mircea");

    sqlite3_close(clients_db);
    return 0;
}