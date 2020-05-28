#include "remote.h"


Socket::Socket(char* OSname) {
    OS_name = OSname;
    // create a new ssh session
    red_pitaya_session = ssh_new();
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

void Socket::setRPIP(char* ip) {
    strcpy(RP_IP, ip);
}

void Socket::setRPName(char* name) {
    strcpy(RP_name, name);
}

void Socket::setRPPassword(char* password) {
    strcpy(RP_password, password);
}

void Socket::saveConfig() {}

void Socket::loadConfig(int number) {}

void Socket::linkStart() {
    int rc;
    // connect session to host
    rc = connect_session(red_pitaya_session, RP_IP, RP_name, RP_password);
    fprintf(stdout, "SSH Session Return %d\n", rc);
}

void Socket::changeFolder(char* folder_name) {
    int rc;
    char command[100] = "cd ";
    char buffer[1024];
    unsigned int nbytes;

    // create shell channel
    shell_channel = ssh_channel_new(red_pitaya_session);
    // use session to connect channel
    rc = connect_channel(shell_channel, red_pitaya_session);
    fprintf(stdout, "Channel Return %d\n", rc);

    strcat(command, folder_name);
    printf("Executing remote command...\n");
    rc = ssh_channel_request_exec(shell_channel, command);

    printf("Received:\n");
    nbytes = ssh_channel_read(shell_channel, buffer, sizeof(buffer), 0);
    while (nbytes > 0) {
        fwrite(buffer, 1, nbytes, stdout);
        nbytes = ssh_channel_read(shell_channel, buffer, sizeof(buffer), 0);
    }
    printf("Has changed to folder %s\n", folder_name);
}

void Socket::listAllFiles() {
    int rc;
    char buffer[1024];
    unsigned int nbytes;

    // create shell channel
    shell_channel = ssh_channel_new(red_pitaya_session);
    // use session to connect channel
    rc = connect_channel(shell_channel, red_pitaya_session);
    fprintf(stdout, "Channel Return %d\n", rc);

    printf("Executing remote command...\n");
    rc = ssh_channel_request_exec(shell_channel, "ls -alh");

    printf("Received:\n");
    nbytes = ssh_channel_read(shell_channel, buffer, sizeof(buffer), 0);
    while (nbytes > 0) {
        fwrite(buffer, 1, nbytes, stdout);
        nbytes = ssh_channel_read(shell_channel, buffer, sizeof(buffer), 0);
    }
}

void Socket::linkTerminated() {
    // close and free channel
    ssh_channel_close(shell_channel);
    ssh_channel_send_eof(shell_channel);
    ssh_channel_free(shell_channel);
    printf("ssh channel closed!\n");
    // close and free session
    ssh_disconnect(red_pitaya_session);
    ssh_free(red_pitaya_session);
    printf("ssh session closed!\n");
}

void Socket::print_result(FILE* fp) {
    char buf[100];
    std::string res;
    int cnt = 0;

    if(!fp)
    {
        return;
    }
    printf("\n>>>\n");
    while(memset(buf, 0, sizeof(buf)), fgets(buf, sizeof(buf) - 1, fp) != 0 ) {
        printf("%s", buf);
        if (cnt == 2)
        {
            res = buf;
        }
        cnt++;
    }
    printf("\n<<<\n");
    int start = res.find("192.");
    int end = res.find("netmask");
    strcpy(computer_IP, (char*)res.substr(start, end-start-2).c_str());
}

int Socket::connect_session(ssh_session session, char *hostname, char *username, char *password) {
    int rc;

    if (session == NULL) {
        fprintf(stderr, "Error creating new ssh session");
        exit(-1);
    }

    // set options
    ssh_options_set(session, SSH_OPTIONS_HOST, hostname);
    ssh_options_set(session, SSH_OPTIONS_USER, username);

    // connect to server
    rc = ssh_connect(session);
    if (rc != SSH_OK) {
        fprintf(stderr, "Error connecting to host: %s\n", ssh_get_error(session));
        ssh_free(session);
        exit(-1);
    }

    // authenticate using password
    rc = ssh_userauth_password(session, NULL, password);
    if (rc != SSH_AUTH_SUCCESS) {
        fprintf(stderr, "Error authenticating with password: %s\n", ssh_get_error(session));
        ssh_disconnect(session);
        ssh_free(session);
        exit(-1);
    }

    return rc;
}

int Socket::connect_channel (ssh_channel channel, ssh_session session) {
    int rc;
    if (channel == NULL) {
        fprintf(stderr, "Error creating channel: %s\n", ssh_get_error(channel));
        ssh_channel_free(channel);
        exit(-1);
    }

    // connect shell channel
    rc = ssh_channel_open_session(channel);
    if (rc != SSH_OK) {
        fprintf(stderr, "Error opening session in channel: %s\n", ssh_get_error(session));
        ssh_channel_free(channel);
        exit(-1);
    }
    return rc;
}