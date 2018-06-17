#include <iostream>
#include "Connection.h"

#define PORT 5600
int main() {
    std::cout << "Hello, World!" << std::endl;
    Connection connection;
    connection.createConnection(PORT);
    return 0;
}