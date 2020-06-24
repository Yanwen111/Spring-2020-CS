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

void Socket::saveConfig() {
    std::string config_name = "config_file/ssh_config_" + std::to_string(number) + ".txt";
    std::ofstream fileout(config_name, std::ios::trunc|std::ios::out);
//    for (auto s: scan_data){
//        fileout << s.quaternion[0] << ' ' << s.quaternion[1] << ' ' << s.quaternion[2] << ' ' << s.quaternion[3] << std::endl;
//    }
    fileout << "IP address: " << RP_IP << std::endl;
    fileout << "User name: " << RP_name << std::endl;
    fileout << "Password: " << RP_password << std::endl;
    fileout.close();
    printf("Config file No.%d has been saved!\n", number);
    number ++;
}

void Socket::loadConfig(int number) {
    std::string stemp;
    std::string config_name = "config_file/ssh_config_" + std::to_string(number) + ".txt";

    std::ifstream filein(config_name);

    filein.getline(RP_IP, 100);
    stemp = RP_IP;
    strcpy(RP_IP, stemp.substr(12).c_str());

    filein.getline(RP_name, 100);
    stemp = RP_name;
    strcpy(RP_name, stemp.substr(11).c_str());

    filein.getline(RP_password, 100);
    stemp = RP_password;
    strcpy(RP_password, stemp.substr(10).c_str());

    filein.close();
    printf("Config file No.%d has been loaded!\n", number);
}

int Socket::remove_cachefile() {
    if (strcmp(OS_name, "Linux") == 0 || strcmp(OS_name, "MacOS") == 0) // change the equation!
        return system("rm data/tempr*.dat");
    else if (strcmp(OS_name, "Windows") == 0)
        return system("del data/tempr*.dat");
}

int Socket::save_datafile(char* newfilename) {
    if(rename("data/tempr.dat", newfilename) == -1)
        throw(std::runtime_error("Error saving and renaming data file"));
    return 1;
}

int Socket::linkStart() {
    int rc;
    // connect session to host
    rc = connect_session(red_pitaya_session, RP_IP, RP_name, RP_password);
    fprintf(stdout, "SSH Session Return %d\n", rc);
    // create shell channel
    shell_channel = ssh_channel_new(red_pitaya_session);
    // use session to connect channel
    rc = connect_channel(shell_channel, red_pitaya_session);
    fprintf(stdout, "Channel Return %d\n", rc);

    rc = ssh_channel_request_pty(shell_channel);
    if (rc != SSH_OK) return rc;
    rc = ssh_channel_change_pty_size(shell_channel, 80, 24);
    if (rc != SSH_OK) return rc;
    rc = ssh_channel_request_shell(shell_channel);
    if (rc != SSH_OK) return rc;
    return 0;
}

int Socket::interactiveShell() {
    int rc;
    char buffer[256];
    int nbytes, nwritten;

    while (ssh_channel_is_open(shell_channel) && !ssh_channel_is_eof(shell_channel))
    {
        nbytes = ssh_channel_read_nonblocking(shell_channel, buffer, sizeof(buffer), 0);
        if (nbytes < 0) return SSH_ERROR;
        if (nbytes > 0)
        {
            nwritten = write(1, buffer, nbytes);
            if (nwritten != nbytes) return SSH_ERROR;
        }
        if (!kbhit())
        {
            usleep(50000L); // 0.05 second
            continue;
        }

        nbytes = read(0, buffer, sizeof(buffer));
        if (nbytes < 0) return SSH_ERROR;
        if (nbytes > 0)
        {
            nwritten = ssh_channel_write(shell_channel, buffer, nbytes);
            if (nwritten != nbytes) return SSH_ERROR;
        }
    }
    return rc;
}

void Socket::changeFolder(char* folder_name) {
    int rc;
    char command[100] = "cd ";
    char buffer[1024];
    unsigned int nbytes, nwritten;

    strcat(command, folder_name);
    strcat(command, "\n");
    printf("Executing remote command...\n");
    //rc = ssh_channel_request_exec(shell_channel, command);

    nbytes = ssh_channel_read_nonblocking(shell_channel, buffer, sizeof(buffer), 0);
    while (nbytes>0)
    {
        fwrite(buffer, 1, nbytes, stdout);
        nbytes = ssh_channel_read_nonblocking(shell_channel, buffer, sizeof(buffer), 0);
    }

    usleep(50000L);
    nwritten = ssh_channel_write(shell_channel, command, sizeof(command));
    usleep(50000L);
    nbytes = ssh_channel_read_nonblocking(shell_channel, buffer, sizeof(buffer), 0);
    usleep(50000L);
    while (nbytes > 0)
    {
        fwrite(buffer, 1, nbytes, stdout);
        nbytes = ssh_channel_read_nonblocking(shell_channel, buffer, sizeof(buffer), 0);
        usleep(5000L);
    }

    printf("\nHas changed to folder %s\n", folder_name);
}

void Socket::listAllFiles() {
    int rc;
    char buffer[1024];
    unsigned int nbytes, nwritten;
    char command[9] = "ls -alh\n";

    nbytes = ssh_channel_read_nonblocking(shell_channel, buffer, sizeof(buffer), 0);
    while (nbytes>0)
    {
        fwrite(buffer, 1, nbytes, stdout);
        nbytes = ssh_channel_read_nonblocking(shell_channel, buffer, sizeof(buffer), 0);
    }

    usleep(50000L);
    nwritten = ssh_channel_write(shell_channel, command, sizeof(command));
    usleep(50000L);
    nbytes = ssh_channel_read_nonblocking(shell_channel, buffer, sizeof(buffer), 0);
    usleep(50000L);
    while (nbytes > 0)
    {
        fwrite(buffer, 1, nbytes, stdout);
        nbytes = ssh_channel_read_nonblocking(shell_channel, buffer, sizeof(buffer), 0);
        usleep(5000L);
    }
}

void Socket::customCommand(char* command, int ms, std::string &output) {
    int rc;
    char buffer[1024];
    unsigned int nbytes, nwritten;
    char command1[500] = "";

    strcat(command1, command);
    strcat(command1, "\n");
    //printf("%s\n", command1);

    nbytes = ssh_channel_read_nonblocking(shell_channel, buffer, sizeof(buffer), 0);
    while (nbytes>0)
    {
        //fwrite(buffer, 1, nbytes, stdout);
        nbytes = ssh_channel_read_nonblocking(shell_channel, buffer, sizeof(buffer), 0);
    }

    usleep(50000L); /* 50 ms*/
    nwritten = ssh_channel_write(shell_channel, command1, sizeof(command1));
    usleep(50000L);
    memset(buffer, 0, sizeof(buffer));
    output.clear();
    nbytes = ssh_channel_read_timeout(shell_channel, buffer, sizeof(buffer), 0, 500);
    output = buffer;
    output.erase(std::remove(output.begin(), output.end(), '^'), output.end());
    output.erase(std::remove(output.begin(), output.end(), '@'), output.end());
    usleep(50000L);
    while (nbytes > 0)
    {
        //fwrite(buffer, 1, nbytes, stdout);
        printf("%s\n", output.c_str());
        memset(buffer, 0, sizeof(buffer));
        output.clear();
        nbytes = ssh_channel_read_timeout(shell_channel, buffer, sizeof(buffer), 0, ms);
        output = buffer;
        output.erase(std::remove(output.begin(), output.end(), '^'), output.end());
        output.erase(std::remove(output.begin(), output.end(), '@'), output.end());
        usleep(5000L);
    }
    //sleep(1);

    printf("\n^v^v^v^v^v^\n");
}

void Socket::multiCommands() {
    //customCommand("cd IOT_project/sshtest", 500);
    //customCommand("./test");
    return;
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
//        exit(-1);
        throw(std::runtime_error("Error creating new ssh session"));
    }

    // set options
    ssh_options_set(session, SSH_OPTIONS_HOST, hostname);
    ssh_options_set(session, SSH_OPTIONS_USER, username);

    // connect to server
    rc = ssh_connect(session);
    if (rc != SSH_OK) {
//        fprintf(stderr, "Error connecting to host: %s\n", ssh_get_error(session));
//        ssh_free(session);
//        return -1;
        std::string errorMessage = std::string("Error connecting to host: ") + ssh_get_error(session);
        ssh_free(session);
        fprintf(stderr, "%s", errorMessage.c_str());
        throw(std::runtime_error(errorMessage));
    }

    // authenticate using password
    rc = ssh_userauth_password(session, NULL, password);
    if (rc != SSH_AUTH_SUCCESS) {
//        fprintf(stderr, "Error authenticating with password: %s\n", ssh_get_error(session));
        std::string errorMessage = std::string("Error authenticating with password: ") + ssh_get_error(session);
        ssh_disconnect(session);
        ssh_free(session);
        fprintf(stderr, "%s", errorMessage.c_str());
        throw(std::runtime_error(errorMessage));
//        exit(-1);
    }

    return rc;
}

int Socket::connect_channel (ssh_channel channel, ssh_session session) {
    int rc;
    if (channel == NULL) {
        std::string errorMessage = std::string("Error creating channel: ") + ssh_get_error(channel);
//        fprintf(stderr, "Error creating channel: %s\n", ssh_get_error(channel));
        ssh_channel_free(channel);
        fprintf(stderr, "%s", errorMessage.c_str());
        throw(std::runtime_error(errorMessage));
//        exit(-1);
    }

    // connect shell channel
    rc = ssh_channel_open_session(channel);
    if (rc != SSH_OK) {
        //        fprintf(stderr, "Error opening session in channel: %s\n", ssh_get_error(session));
        std::string errorMessage = std::string("Error opening session in channel: ") + ssh_get_error(session);
        ssh_channel_free(channel);
        fprintf(stderr, "%s", errorMessage.c_str());
        throw(std::runtime_error(errorMessage));
    }
    return rc;
}

int Socket::kbhit() {
    struct timeval tv = { 0L, 0L };
    fd_set fds;

    FD_ZERO(&fds);
    FD_SET(0, &fds);

    return select(1, &fds, NULL, NULL, &tv);
}