// Jeel Jadawala - 110096762 | Hardi Kadia - 110097496
// Section 1

// mirror.c

#include <stdio.h>

#include <stdlib.h>

#include <string.h>

#include <unistd.h>

#include <sys/socket.h>

#include <arpa/inet.h>

#include <netinet/in.h>

#include <sys/wait.h>

#include <dirent.h>

#include <sys/stat.h>

#include <time.h>

#include <fcntl.h>

#define GRP7_SERV_IPADD "127.0.0.1"

#define GRP7_MX_CONN 12 // Maximum number of clients (including server and mirror)

#define GRP7_BUFSIZ 10000 // Buffer size for receiving data from clients

#define GRP7_SERVPORT 8081

#define GRP7_MX_grp7_filelocSIZ 4096

#define GRP7_CMMDSIZ 10000

#define GRP7_MXARG 10

#define GRP7_MX_CMMDLEN 2048

FILE *grp7_flpt;

char grp7_flbuff[1024] = {0};

void processclient(int grp7_cltsockt);

// void callMirror(int grp7_cltsockt);

// grp7_cmmds to be run by the server

// srch_for_fls

// when calling, set initial grp7_fileloc to begin with home

void srch_for_fl(int grp7_cltsockt, char *grp7_flname, char *grp7_fileloc, int *grp7_isFileFound)

{

    DIR *dir;

    struct dirent *grp7_dP;

    struct stat st;

    char buf[GRP7_BUFSIZ];

    // Open the directory at the specified grp7_fileloc

    dir = opendir(grp7_fileloc);

    if (dir == NULL)

    {

        perror("error : directory is null");

        return;
    }

    // Traverse the directory

    while ((grp7_dP = readdir(dir)) != NULL)

    {

        // Check if the current directory grp7_entry is the target file

        if (strcmp(grp7_dP->d_name, grp7_flname) == 0)

        {

            // Construct the full grp7_fileloc to the file

            sprintf(buf, "%s/%s", grp7_fileloc, grp7_flname);

            // Get the file stats

            if (stat(buf, &st) == 0)

            {

                // Print the file information

                printf("filepath:%s\n filename :%s\n File Size:%ld\nCreated At:%s\n", grp7_fileloc, grp7_flname, st.st_size, ctime(&st.st_mtime));

                char grp7_msg[1000];

                memset(grp7_msg, 0, sizeof(grp7_msg));

                sprintf(grp7_msg, "filepath:%s\n filename :%s\n File Size:%ld\nCreated At:%s\n", grp7_fileloc, grp7_flname, st.st_size, ctime(&st.st_mtime));

                grp7_msg[strlen(grp7_msg)] = '\0';

                if (send(grp7_cltsockt, grp7_msg, strlen(grp7_msg), 0) < 0)

                {

                    perror("send failed");

                    exit(EXIT_FAILURE);
                }

                memset(grp7_msg, 0, sizeof(grp7_msg));

                // Set the grp7_isFileFound flag

                *grp7_isFileFound = 1;

                // return grp7_msg;

                // Break out of the loop

                break;
            }
        }

        // Recursively search subdirectories

        if (grp7_dP->d_type == DT_DIR && strcmp(grp7_dP->d_name, ".") != 0 && strcmp(grp7_dP->d_name, "..") != 0)

        {

            // Construct the full grp7_fileloc to the subdirectory

            sprintf(buf, "%s/%s", grp7_fileloc, grp7_dP->d_name);

            // Recursively search the subdirectory

            srch_for_fl(grp7_cltsockt, grp7_flname, buf, grp7_isFileFound);

            // If the file is grp7_isFileFound, break out of the loop

            if (*grp7_isFileFound)

            {

                break;
            }
        }
    }

    // Close the directory

    closedir(dir);
}

void make_tar_by_size(const char *dir_grp7_fileloc, const char *tar_grp7_fileloc, const char *grp7_firstsize, const char *grp7_secondsize)

{

    char grp7_cmmd[GRP7_MX_CMMDLEN];

    snprintf(grp7_cmmd, GRP7_MX_CMMDLEN, "find %s -type f -size +%sc -a -size -%sc -print0 | tar czvf %s --null -T - >/dev/null 2>&1", dir_grp7_fileloc, grp7_firstsize, grp7_secondsize, tar_grp7_fileloc);

    system(grp7_cmmd);
}

void make_tar_by_date(const char *dir_grp7_fileloc, const char *tar_grp7_fileloc, const char *grp7_firstdate, const char *grp7_seconddate)

{

    char grp7_cmmd[GRP7_MX_CMMDLEN];

    snprintf(grp7_cmmd, GRP7_MX_CMMDLEN, "find %s -type f -newermt %s ! -newermt %s -print | tar czvf %s -T - >/dev/null 2>&1", dir_grp7_fileloc, grp7_firstdate, grp7_seconddate, tar_grp7_fileloc);

    system(grp7_cmmd);
}

// make_tar_by_file

void look_for_directory(char *dir_grp7_fileloc, char **files, int grp7_flcnt, char *complete_files_grp7_fileloc)

{

    DIR *dir = opendir(dir_grp7_fileloc);

    if (!dir)

    {

        perror("Failed to open directory");

        exit(1);
    }

    struct dirent *grp7_entry;

    char file_grp7_fileloc[GRP7_MX_grp7_filelocSIZ];

    while ((grp7_entry = readdir(dir)) != NULL)

    {

        if (grp7_entry->d_type == DT_DIR)

        {

            if (strcmp(grp7_entry->d_name, ".") == 0 || strcmp(grp7_entry->d_name, "..") == 0)

            {

                continue;
            }

            sprintf(file_grp7_fileloc, "%s/%s", dir_grp7_fileloc, grp7_entry->d_name);

            look_for_directory(file_grp7_fileloc, files, grp7_flcnt, complete_files_grp7_fileloc);
        }

        else if (grp7_entry->d_type == DT_REG)

        {

            for (int i = 0; i < grp7_flcnt; i++)

            {

                if (strcmp(files[i], grp7_entry->d_name) == 0)

                {

                    char file_grp7_fileloc[GRP7_MX_grp7_filelocSIZ];

                    sprintf(file_grp7_fileloc, "%s/%s", dir_grp7_fileloc, grp7_entry->d_name);

                    sprintf(complete_files_grp7_fileloc, "%s%s%s", complete_files_grp7_fileloc, file_grp7_fileloc, " ");
                }
            }
        }
    }

    closedir(dir);
}

void make_tar_by_file(char *tar_grp7_flname, char **files, int grp7_flcnt, int grp7_isFileFound)

{

    char complete_files_grp7_fileloc[GRP7_CMMDSIZ] = ""; // Initialize to empty string

    char grp7_cmmd[GRP7_CMMDSIZ];

    char file_grp7_fileloc[GRP7_MX_grp7_filelocSIZ];

    look_for_directory(getenv("HOME"), files, grp7_flcnt, complete_files_grp7_fileloc);

    if (strlen(complete_files_grp7_fileloc) == 0)

    {

        grp7_isFileFound = 0;

        printf("No file grp7_isFileFound\n");

        exit(0);
    }

    else

    {

        sprintf(grp7_cmmd, "%s%s%s", "tar -czvf temp.tar.gz -P", " ", complete_files_grp7_fileloc);

        system(grp7_cmmd);
    }
}

// validity checker for gettarz

int check_for_extensionName(char *grp7_flname, char **grp7_ext, int grp7_extCount)

{

    char *dot_pos = strrchr(grp7_flname, '.');

    if (!dot_pos)

    {

        return 0;
    }

    char *extension = dot_pos + 1;

    for (int i = 0; i < grp7_extCount; i++)

    {

        if (strcmp(extension, grp7_ext[i]) == 0)

        {

            return 1;
        }
    }

    return 0;
}

void searchAdd_file_by_extension(char *root_grp7_fileloc, char *tar_grp7_flname, char **grp7_ext, int grp7_extCount, char *complete_files_grp7_fileloc)

{

    DIR *grp7_base_dir = opendir(root_grp7_fileloc);

    if (!grp7_base_dir)

    {

        perror("Failed to open directory");

        exit(1);
    }

    struct dirent *dir_grp7_entry;

    while ((dir_grp7_entry = readdir(grp7_base_dir)) != NULL)

    {

        char file_grp7_fileloc[GRP7_MX_grp7_filelocSIZ];

        sprintf(file_grp7_fileloc, "%s/%s", root_grp7_fileloc, dir_grp7_entry->d_name);

        if (dir_grp7_entry->d_type == DT_DIR && strcmp(dir_grp7_entry->d_name, ".") != 0 && strcmp(dir_grp7_entry->d_name, "..") != 0)

        {

            searchAdd_file_by_extension(file_grp7_fileloc, tar_grp7_flname, grp7_ext, grp7_extCount, complete_files_grp7_fileloc);
        }

        else if (dir_grp7_entry->d_type == DT_REG && check_for_extensionName(dir_grp7_entry->d_name, grp7_ext, grp7_extCount))

        {

            printf("file: %s\n", file_grp7_fileloc);

            sprintf(complete_files_grp7_fileloc, "%s%s%s", complete_files_grp7_fileloc, file_grp7_fileloc, " ");
        }
    }

    closedir(grp7_base_dir);
}

// gettarz

void make_tar_by_ext(char *tar_grp7_flname, char **grp7_ext, int grp7_extCount)

{

    char complete_files_grp7_fileloc[50000] = "";

    char grp7_cmmd[GRP7_CMMDSIZ] = "";

    searchAdd_file_by_extension(getenv("HOME"), tar_grp7_flname, grp7_ext, grp7_extCount, complete_files_grp7_fileloc);

    if (strlen(complete_files_grp7_fileloc) == 0)

    {

        printf("No files found\n");

        exit(0);
    }

    else

    {

        sprintf(grp7_cmmd, "%s%s%s", "tar -czvf temp.tar.gz -P", " ", complete_files_grp7_fileloc);

        printf("complete file grp7_filelocs: %s\n", complete_files_grp7_fileloc);

        printf("grp7_cmmd: %s\n", grp7_cmmd);

        system(grp7_cmmd);
    }
}

// send file to client

int pass_file_by_byte(int grp7_socFd, char *grp7_flpt)

{

    int grp7_procID = fork();

    if (grp7_procID == 0)

    {

        int grp7_flFd, len;

        fprintf(stderr, "Copying file to client %d\n", grp7_socFd);

        grp7_flFd = open(grp7_flpt, O_RDONLY);

        char buffer[GRP7_BUFSIZ];

        memset(buffer, 0, sizeof(buffer));

        while ((len = read(grp7_flFd, buffer, GRP7_BUFSIZ)) > 0)

        {

            if (send(grp7_socFd, buffer, len, 0) == -1)

            {

                perror("send");

                return -1;
            }
        }

        memset(buffer, 0, sizeof(buffer));

        sleep(1);

        fprintf(stderr, "File transfer to client successfully\n");

        close(grp7_flFd);

        close(grp7_socFd);

        exit(0);
    }

    wait(NULL);

    return 1;
}

void processclient(int grp7_cltsockt)

{

    char buffer[GRP7_BUFSIZ];

    int file_fd;

    // Loop to receive and send grp7_msgs

    char client_grp7_msg[2000];

    char grp7_cmmd[1000], server_reply[2000];

    char *grp7_arguments[GRP7_MXARG];

    int grp7_argCount;

    while (1)

    {

        // Receive grp7_msg from client

        int read_size = recv(grp7_cltsockt, client_grp7_msg, 2000, 0);

        if (read_size == 0)

        {
            // ctr--;
            printf("Client disconnected\n");
            //  printf("Active clients %d\n", ctr);
            break;
        }

        // Add null terminator to grp7_msg

        client_grp7_msg[read_size] = '\0';

        printf("Client grp7_msg: %s", client_grp7_msg);

        char grp7_tmpvar[1000];

        sprintf(grp7_tmpvar, "%s", client_grp7_msg);

        // parsing

        grp7_argCount = 0;

        grp7_arguments[grp7_argCount] = strtok(grp7_tmpvar, " \n");

        int grp7_flTransfer = 0;

        while (grp7_arguments[grp7_argCount] != NULL && grp7_argCount < GRP7_MXARG - 1)

        {

            grp7_argCount++;

            grp7_arguments[grp7_argCount] = strtok(NULL, " \n");
        }
        int grp7_cmmd_success_flag = 0;

        char grp7_msg[1000];

        if (strcmp(grp7_arguments[0], "filesrch") == 0)

        {

            printf("filesrch invoked\n");

            grp7_flTransfer = 0;

            char *grp7_flname = grp7_arguments[1];

            char *grp7_fileloc = getenv("HOME");

            int grp7_isFileFound = 0;

            srch_for_fl(grp7_cltsockt, grp7_flname, grp7_fileloc, &grp7_isFileFound);

            if (grp7_isFileFound == 0)

            {

                printf("file not present\n");

                memset(grp7_msg, 0, sizeof(grp7_msg));

                sprintf(grp7_msg, "file not present\n");

                grp7_msg[strlen(grp7_msg)] = '\0';

                if (send(grp7_cltsockt, grp7_msg, strlen(grp7_msg), 0) < 0)

                {

                    perror("failure in sending tar file");

                    exit(EXIT_FAILURE);
                }

                memset(grp7_msg, 0, sizeof(grp7_msg));
            }
        }

        else if (strcmp(grp7_arguments[0], "tarfgetz") == 0)

        {

            printf("tarfgetz invoked\n");

            grp7_flTransfer = 1;

            long grp7_firstsize = atol(grp7_arguments[1]);

            long grp7_secondsize = atol(grp7_arguments[2]);

            if (grp7_firstsize < 0 || grp7_secondsize < 0 || grp7_firstsize > grp7_secondsize)

            {

                printf("Invalid size parameters\n");

                return 0;
            }

            char grp7_homeDir[1024];

            snprintf(grp7_homeDir, sizeof(grp7_homeDir), "%s", getenv("HOME"));

            char grp7_firstsize_str[20]; // Adjust the size as needed

            char grp7_secondsize_str[20]; // Adjust the size as needed

            snprintf(grp7_firstsize_str, sizeof(grp7_firstsize_str), "%ld", grp7_firstsize);

            snprintf(grp7_secondsize_str, sizeof(grp7_secondsize_str), "%ld", grp7_secondsize);

            printf("Home Directory: %s\n", grp7_homeDir);

            printf("Size Range: %s - %s\n", grp7_firstsize_str, grp7_secondsize_str);

            make_tar_by_size(grp7_homeDir, "temp.tar.gz", grp7_firstsize_str, grp7_secondsize_str);

            grp7_flTransfer = 1;
        }

        else if (strcmp(grp7_arguments[0], "getdirf") == 0)

        {

            struct tm grp7_firstdate_tm, grp7_seconddate_tm;

            printf("getdirf invoked\n");

            memset(&grp7_firstdate_tm, 0, sizeof(grp7_firstdate_tm));

            memset(&grp7_seconddate_tm, 0, sizeof(grp7_seconddate_tm));

            if (strptime(grp7_arguments[1], "%Y-%m-%d", &grp7_firstdate_tm) == NULL || strptime(grp7_arguments[2], "%Y-%m-%d", &grp7_seconddate_tm) == NULL)

            {

                printf("Invalid date format: YYYY-MM-DD\n");
            }

            else

            {

                char grp7_homeDir[1024];

                snprintf(grp7_homeDir, 1024, "%s", getenv("HOME"));

                time_t grp7_firstdate = mktime(&grp7_firstdate_tm);

                time_t grp7_seconddate = mktime(&grp7_seconddate_tm);

                char formatted_grp7_firstdate[20];

                char formatted_grp7_seconddate[20];

                strftime(formatted_grp7_firstdate, sizeof(formatted_grp7_firstdate), "%Y-%m-%d", &grp7_firstdate_tm);

                strftime(formatted_grp7_seconddate, sizeof(formatted_grp7_seconddate), "%Y-%m-%d", &grp7_seconddate_tm);

                make_tar_by_date(grp7_homeDir, "temp.tar.gz", formatted_grp7_firstdate, formatted_grp7_seconddate);

                grp7_flTransfer = 1;
            }
        }

        else if (strcmp(grp7_arguments[0], "fgets") == 0)

        {

            printf("fgets invoked\n");

            if (grp7_argCount < 2 || grp7_argCount > 5)

            {

                printf("Invalid number of arguments. Usage: fget <file1> <file2> ... <file4>\n");
            }

            else

            {

                char **files = malloc((grp7_argCount - 1) * sizeof(char *));

                for (int i = 1; i < grp7_argCount; i++)

                {

                    files[i - 1] = grp7_arguments[i];
                }

                int grp7_flcnt = grp7_argCount - 1;

                if (grp7_flcnt > 4)

                {

                    printf("Error: Maximum of 4 files allowed.\n");

                    free(files);
                }

                else

                {

                    char tar_grp7_flname[] = "temp.tar.gz";

                    int grp7_isFileFound = 0;

                    make_tar_by_file(tar_grp7_flname, files, grp7_flcnt, &grp7_isFileFound);

                    grp7_flTransfer = 1;
                }
            }
        }

        else if (strcmp(grp7_arguments[0], "targzf") == 0)

        {

            if (grp7_argCount < 2 || grp7_argCount > 6)
            {
                printf("Invalid number of arguments for targzf.\n");
                return;
            }

            printf("targzf invoked\n");

            char **grp7_ext = malloc((grp7_argCount - 1) * sizeof(char *));

            for (int i = 1; i < grp7_argCount; i++)
            {
                grp7_ext[i - 1] = grp7_arguments[i];
            }

            int grp7_extCount = grp7_argCount - 1;

            if (grp7_argCount == 6 && strcmp(grp7_arguments[5], "-u") == 0)
            {
                grp7_extCount--;
            }

            if (grp7_extCount < 1 || grp7_extCount > 4)
            {
                printf("maximum extension should be 4\n");
                free(grp7_ext);
                return;
            }

            char tar_grp7_flname[] = "temp.tar.gz";

            make_tar_by_ext(tar_grp7_flname, grp7_ext, grp7_extCount);

            grp7_flTransfer = 1;
        }

        else if (strcmp(grp7_arguments[0], "quit") == 0)

        {

            printf("Client requested to exit\n");

            break;
        }

        if (grp7_flTransfer)

        {

            printf("Transfering file....\n");

            pass_file_by_byte(grp7_cltsockt, "temp.tar.gz");
        }
    }

    close(grp7_cltsockt);

    // fclose(grp7_flpt);
}

int main()
{
    int grp7_servSocket, grp7_cltsockt;

    struct sockaddr_in grp7_servAddr, grp7_cltAddr;

    socklen_t grp7_cltAddr_len;

    int grp7_clients_track = 0;

    int opt = 1;

    // Socket creation
    grp7_servSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (grp7_servSocket == -1)
    {
        perror("Error: Failed to create socket");
        exit(1);
    }

    //clear memory buffer 
    memset(&grp7_servAddr, 0, sizeof(grp7_servAddr));


    // Server address configuration
    grp7_servAddr.sin_family = AF_INET;
    grp7_servAddr.sin_addr.s_addr = inet_addr(GRP7_SERV_IPADD); // INADDR_ANY;
    grp7_servAddr.sin_port = htons(GRP7_SERVPORT); 

    // Bind the server address to the socket
    if (bind(grp7_servSocket, (struct sockaddr *)&grp7_servAddr, sizeof(grp7_servAddr)) == -1)
    {
        perror("Error: Failed to bind socket");
        exit(1);
    }

    // Listen to connections
    if (listen(grp7_servSocket, GRP7_MX_CONN) == -1)
    {
        perror("Error: Failed to listen");
        exit(1);
    }

    printf("Mirror waiting for client...\n");

    //continuous loop for client mirror communication
    while (1)
    {
    grp7_cltAddr_len = sizeof(grp7_cltAddr);
        grp7_cltsockt = accept(grp7_servSocket, (struct sockaddr *)&grp7_cltAddr, &grp7_cltAddr_len); // accept the valid connections
        if (grp7_cltsockt == -1)

        {

            perror("Error: Failed to accept connection");

            exit(1);
        }

        printf("\nClient connection established at: %s:%d\n", inet_ntoa(grp7_cltAddr.sin_addr), ntohs(grp7_cltAddr.sin_port));

        pid_t grp7_procID = fork();

        if (grp7_procID == -1)

        {

            close(grp7_cltsockt);

            perror("Error: Could not fork to handle client");

            continue;
        }

        else if (grp7_procID == 0)

        {

            char welcome_message[] = "Welcome to the mirror!";
            if (send(grp7_cltsockt, welcome_message, strlen(welcome_message), 0) == -1)
            {
                perror("Error: Failed to send welcome message to client");
                exit(EXIT_FAILURE);
            }

            // Child process

            printf("Client %d handled by the mirror\n", grp7_clients_track + 1);
            processclient(grp7_cltsockt);
            exit(EXIT_SUCCESS);
        }

        else

        {

            grp7_clients_track++; // Increment count of clients.

            close(grp7_cltsockt); // Close unused client socket
        }
    }

    return 0;
}