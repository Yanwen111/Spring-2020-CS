#include <iostream>
#include <libssh/libssh.h>
#include <string.h>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <cstring>


class Socket {
public:
    Socket(char* OSname);
    //char *hostname = (char*)"192.168.1.42";
    char* OS_name = (char*)"Linux";
    char* computer_IP;
    char* RP_IP;
    char* RP_name;
    char* RP_password;

    void getComputerIP();
    void setRPIP();
    void setRPName();
    void setRPPassword();
    void print_result(FILE *fp);

    void saveConfig();
    void loadConfig(int number);

private:
    int number = 0;

};

