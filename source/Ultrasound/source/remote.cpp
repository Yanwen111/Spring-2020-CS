#include "remote.h"


Socket::Socket(char* OSname) {
    OS_name = OSname;
}

void Socket::getComputerIP() {
    char* command;
    if (strcmp(OS_name, "Linux") == 0 || strcmp(OS_name, "MacOS") == 0) // change the equation!
        command = (char*)"ifconfig | grep inet";
    else if (strcmp(OS_name, "Windows") == 0)
        command = (char*)"ipconfig | findstr IPv4";
    else
    {
        printf("%s is not a proper OS name ! (MacOS, Linux, Windows) \n", OS_name);
        return;
    }

    char* res; /* origin result after send the command */
    FILE *fp = NULL;
    fp = popen(command, "r");
    if(!fp) {
        perror("popen");
        exit(EXIT_FAILURE);
    }
    print_result(fp);
}

void Socket::setRPIP() {}

void Socket::setRPName() {}

void Socket::setRPPassword() {}

void Socket::saveConfig() {}

void Socket::loadConfig(int number) {}

void Socket::print_result(FILE* fp) {
    char buf[100];

    if(!fp)
    {
        return;
    }
    printf("\n>>>\n");
    while(memset(buf, 0, sizeof(buf)), fgets(buf, sizeof(buf) - 1, fp) != 0 ) {
        printf("%s", buf);
    }
    printf("\n<<<\n");
}