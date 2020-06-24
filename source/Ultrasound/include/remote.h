#include <iostream>
#include <libssh/libssh.h>
#include <string.h>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <cstring>
#include <string>
#include <fstream>
#include <algorithm>


class Socket {
public:
    Socket(char* OSname);
    //char *hostname = (char*)"192.168.1.42";
    ssh_session red_pitaya_session;
    ssh_channel shell_channel;
    char* OS_name = (char*)"Linux";
    char* computer_IP = new char[30];
    char* RP_IP = new char[30];
    char* RP_name = new char[30];
    char* RP_password = new char[30];

    void getComputerIP();
    void setRPIP(char* ip);
    void setRPName(char* name);
    void setRPPassword(char* password);

    void saveConfig();
    void loadConfig(int number);
    int remove_cachefile();
    int save_datafile(char* newfilename); /* 0 for success, -1 for fail */

    int linkStart();
    void changeFolder(char* folder_name); /* IOT_Project */
    void listAllFiles();
    void customCommand(char* command, int ms, std::string &output);
    void multiCommands();
    int interactiveShell();
    void linkTerminated();

private:
    int number = 0;
    void print_result(FILE *fp);
    int connect_session(ssh_session session, char *hostname, char *username, char *password);
    int connect_channel (ssh_channel channel, ssh_session session);
    int kbhit();/* Under Linux, it determines whether a key has been pressed. Under Windows, it is a standard function */

};