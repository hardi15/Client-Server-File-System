// Jeel Jadawala - 110096762 | Hardi Kadia - 110097496
// Section 1

// server.c

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

#define GRP7_MX_CONN 12 

#define GRP7_BUFSIZ 10000 

#define GRP7_SERVPORT 3490

#define GRP7_MX_grp7_filelocSIZ 4096

#define GRP7_CMMDSIZ 10000

#define GRP7_MXARG 10

#define GRP7_MX_CMMDLEN 2048


//For reading/writing files, a file pointer and buffer are used.
FILE *grp7_flpt;

char grp7_flbuff[1024] = {0};

void processclient(int grp7_cltsockt);


//Find a file in a directory and inform the client
void srch_for_fl(int grp7_cltsockt, char *grp7_flname, char *grp7_fileloc, int *grp7_isFileFound)

{

    DIR *dir;

    struct dirent *grp7_dP;

    struct stat st;

    char buf[GRP7_BUFSIZ];

    //open directory to search through it
    dir = opendir(grp7_fileloc);

    if (dir == NULL)

    {

        perror("error : directory is null");

        return;
    }

    while ((grp7_dP = readdir(dir)) != NULL){

        if (strcmp(grp7_dP->d_name, grp7_flname) == 0)

        {

            sprintf(buf, "%s/%s", grp7_fileloc, grp7_flname);


            if (stat(buf, &st) == 0)

            {

                printf("filepath:%s\n filename :%s\n File Size:%ld\nCreated At:%s\n", grp7_fileloc, grp7_flname, st.st_size, ctime(&st.st_mtime));

                char grp7_msg[1000];

                memset(grp7_msg, 0, sizeof(grp7_msg));

                //printing required info of file
                sprintf(grp7_msg, "filepath:%s\n filename :%s\n File Size:%ld\nCreated At:%s\n", grp7_fileloc, grp7_flname, st.st_size, ctime(&st.st_mtime));

                grp7_msg[strlen(grp7_msg)] = '\0';

                if (send(grp7_cltsockt, grp7_msg, strlen(grp7_msg), 0) < 0)

                {

                    perror("send failed");

                    exit(EXIT_FAILURE);
                }

                memset(grp7_msg, 0, sizeof(grp7_msg));

                *grp7_isFileFound = 1;


                break;
            }
        }

        // Recursively searching subdirectories.
        if (grp7_dP->d_type == DT_DIR && strcmp(grp7_dP->d_name, ".") != 0 && strcmp(grp7_dP->d_name, "..") != 0)

        {


            sprintf(buf, "%s/%s", grp7_fileloc, grp7_dP->d_name);


            srch_for_fl(grp7_cltsockt, grp7_flname, buf, grp7_isFileFound);


            if (*grp7_isFileFound)

            {

                break;
            }
        }
    }


    closedir(dir);
}

// Creates a tar archive with a specified size range.
void make_tar_by_size(const char *dir_grp7_fileloc, const char *tar_grp7_fileloc, const char *grp7_firstsize, const char *grp7_secondsize)

{

    char grp7_cmmd[GRP7_MX_CMMDLEN];

    snprintf(grp7_cmmd, GRP7_MX_CMMDLEN, "find %s -type f -size +%sc -a -size -%sc -print0 | tar czvf %s --null -T - >/dev/null 2>&1", dir_grp7_fileloc, grp7_firstsize, grp7_secondsize, tar_grp7_fileloc);

    system(grp7_cmmd);
}
//hardi
// Creates a tar archive for a specified date period.
void make_tar_by_date(const char *dir_grp7_fileloc, const char *tar_grp7_fileloc, const char *grp7_firstdate, const char *grp7_seconddate)

{

    char grp7_cmmd[GRP7_MX_CMMDLEN];

    snprintf(grp7_cmmd, GRP7_MX_CMMDLEN, "find %s -type f -newermt %s ! -newermt %s -print | tar czvf %s -T - >/dev/null 2>&1", dir_grp7_fileloc, grp7_firstdate, grp7_seconddate, tar_grp7_fileloc);

    system(grp7_cmmd);
}

// Recursively search through folders for specific files 
void look_for_directory(char *dir_grp7_fileloc, char **files, int grp7_flcnt, char *complete_files_grp7_fileloc){
    //open directory to search through it
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
            // Building of path to the subdirectory.
            sprintf(file_grp7_fileloc, "%s/%s", dir_grp7_fileloc, grp7_entry->d_name);

            look_for_directory(file_grp7_fileloc, files, grp7_flcnt, complete_files_grp7_fileloc);
        }

        else if (grp7_entry->d_type == DT_REG)

        {

            for (int i = 0; i < grp7_flcnt; i++)

            {
                // Checking of current file matches any of the specified files.
                if (strcmp(files[i], grp7_entry->d_name) == 0)

                {

                    char file_grp7_fileloc[GRP7_MX_grp7_filelocSIZ];

                    sprintf(file_grp7_fileloc, "%s/%s", dir_grp7_fileloc, grp7_entry->d_name);
                    // Appending the file path to the complete list.
                    sprintf(complete_files_grp7_fileloc, "%s%s%s", complete_files_grp7_fileloc, file_grp7_fileloc, " ");
                }
            }
        }
    }

    closedir(dir);
}
//hardi
// Creates a tar archive with the specified files.
void make_tar_by_file(char *tar_grp7_flname, char **files, int grp7_flcnt, int grp7_isFileFound)
{
    // Initialize variables
    char complete_files_grp7_fileloc[GRP7_CMMDSIZ] = ""; // Store complete file paths
    char grp7_cmmd[GRP7_CMMDSIZ]; // Command string buffer
    char file_grp7_fileloc[GRP7_MX_grp7_filelocSIZ]; // Store a file location

    // Search for specified files in the HOME directory and its subdirectories
    look_for_directory(getenv("HOME"), files, grp7_flcnt, complete_files_grp7_fileloc);

    // Check if any files are found or not
    if (strlen(complete_files_grp7_fileloc) == 0)
    {
        grp7_isFileFound = 0; // Indicate that no files were found
        printf("No file grp7_isFileFound\n");
        exit(0); // Exit the function
    }
    else
    {
        // Create a tar command to archive the found files
        sprintf(grp7_cmmd, "%s%s%s", "tar -czvf temp.tar.gz -P", " ", complete_files_grp7_fileloc);

        // Execute the tar command using the system call
        system(grp7_cmmd);
    }
}


//Checks whether a file has a specific extension
int check_for_extensionName(char *grp7_flname, char **grp7_ext, int grp7_extCount)

{
    // Find the position of the last dot in the file name.
    char *dot_pos = strrchr(grp7_flname, '.');

    if (!dot_pos)

    {

        return 0;
    }

    char *extension = dot_pos + 1; // Extracting of extension

    // Comparing of extension extracted with the list of given extensions.
    for (int i = 0; i < grp7_extCount; i++)

    {

        if (strcmp(extension, grp7_ext[i]) == 0)

        {

            return 1;
        }
    }

    return 0;
}

//h
// Searching for files with specific extensions.
void searchAdd_file_by_extension(char *root_grp7_fileloc, char *tar_grp7_flname, char **grp7_ext, int grp7_extCount, char *complete_files_grp7_fileloc)
{
    // Open the directory
    DIR *grp7_base_dir = opendir(root_grp7_fileloc);
    if (!grp7_base_dir)
    {
        perror("Failed to open directory");
        exit(1);
    }

    // Initialize a structure to hold directory entry
    struct dirent *dir_grp7_entry;

    // Loop through each entry in the directory
    while ((dir_grp7_entry = readdir(grp7_base_dir)) != NULL)
    {
        // Create a complete file path
        char file_grp7_fileloc[GRP7_MX_grp7_filelocSIZ];
        sprintf(file_grp7_fileloc, "%s/%s", root_grp7_fileloc, dir_grp7_entry->d_name);

        // Check if the entry is a directory and not "." or ".."
        if (dir_grp7_entry->d_type == DT_DIR && strcmp(dir_grp7_entry->d_name, ".") != 0 && strcmp(dir_grp7_entry->d_name, "..") != 0)
        {
            // Recursively search directories
            searchAdd_file_by_extension(file_grp7_fileloc, tar_grp7_flname, grp7_ext, grp7_extCount, complete_files_grp7_fileloc);
        }
        else if (dir_grp7_entry->d_type == DT_REG && check_for_extensionName(dir_grp7_entry->d_name, grp7_ext, grp7_extCount))
        {
            // Print and store file path if it matches the extensions
            //printf("file: %s\n", file_grp7_fileloc);
            sprintf(complete_files_grp7_fileloc, "%s%s%s", complete_files_grp7_fileloc, file_grp7_fileloc, " ");
        }
    }

    // Close the directory
    closedir(grp7_base_dir);
}

//h
// Create tar file based on particular file extensions.
void make_tar_by_ext(char *tar_grp7_flname, char **grp7_ext, int grp7_extCount)
{
    // Initialize variables
    char complete_files_grp7_fileloc[50000] = ""; // Store complete file paths
    char grp7_cmmd[GRP7_CMMDSIZ] = ""; // Store the command string

    // Search for files with specific extensions in the HOME directory path
    searchAdd_file_by_extension(getenv("HOME"), tar_grp7_flname, grp7_ext, grp7_extCount, complete_files_grp7_fileloc);

    // Check if any files are found
    if (strlen(complete_files_grp7_fileloc) == 0)
    {
        printf("No files found\n");
        exit(0);
    }
    else
    {
        // Create a tar command to archive the found files
        sprintf(grp7_cmmd, "%s%s%s", "tar -czvf temp.tar.gz -P", " ", complete_files_grp7_fileloc);

        // Print complete file path and the tar command
        //printf("complete file path: %s\n", complete_files_grp7_fileloc);
        //printf("path: %s\n", grp7_cmmd);

        // Execute the tar command using the system call
        system(grp7_cmmd);
    }
}

//h
// Byte-wise transfer to send a file's contents to the client.
// Function to pass a file's content by bytes to a socket
int pass_file_by_byte(int grp7_socFd, char *grp7_flpt)
{
    // Create a child process using fork()
    int grp7_procID = fork();

    // Child process code
    if (grp7_procID == 0)
    {
        int grp7_flFd, len;

        // Print message indicating that the file is being copied to the client
        fprintf(stderr, "Copying file to client %d\n", grp7_socFd);

        // Open the file that needs to be transferred
        grp7_flFd = open(grp7_flpt, O_RDONLY);

        char buffer[GRP7_BUFSIZ];

        // Clear the buffer
        memset(buffer, 0, sizeof(buffer));

        // Read and send the file content in bytes
        while ((len = read(grp7_flFd, buffer, GRP7_BUFSIZ)) > 0)
        {
            // Send the content of the buffer to the socket
            if (send(grp7_socFd, buffer, len, 0) == -1)
            {
                perror("send");
                return -1; // Return -1 on send error
            }
        }

        // Clear the buffer
        memset(buffer, 0, sizeof(buffer));

        // Sleep for a short duration
        sleep(1);

        // Print message indicating successful file transfer to the client
        fprintf(stderr, "File transfer to client successfully\n");

        // Close the file descriptor and socket
        close(grp7_flFd);
        close(grp7_socFd);

        // Exit the child process
        exit(0);
    }

    // Wait for the child process to complete
    wait(NULL);

    // Return 1 to indicate successful file transfer
    return 1;
}


//h
//Process client requests using this function.
void processclient(int grp7_cltsockt)

{

    char buffer[GRP7_BUFSIZ];

    int file_fd;


    char client_grp7_msg[2000];

    char grp7_cmmd[1000], server_reply[2000];

    char *grp7_arguments[GRP7_MXARG];

    int grp7_argCount;

    while (1)

    {


        int read_size = recv(grp7_cltsockt, client_grp7_msg, 2000, 0);

        if (read_size == 0)

        {
            printf("Client disconnected\n");

            break;
        }


        client_grp7_msg[read_size] = '\0';

        printf("Client grp7_msg: %s", client_grp7_msg);

        char grp7_tmpvar[1000];

        sprintf(grp7_tmpvar, "%s", client_grp7_msg);


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

        // Handle various client commands
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

            //check if provided size is as per the requirements or not

            if (grp7_firstsize < 0 || grp7_secondsize < 0 || grp7_firstsize > grp7_secondsize)

            {

                printf("Invalid size parameters\n");

                return 0;
            }

            char grp7_homeDir[1024];

            snprintf(grp7_homeDir, sizeof(grp7_homeDir), "%s", getenv("HOME"));

            char grp7_firstsize_str[20]; 

            char grp7_secondsize_str[20];

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
            //check if user is providing proper arguments or not
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
            // check the arguments are in the given limit or not
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

}



int main()

{
    int grp7_servSocket, grp7_cltsockt;

    struct sockaddr_in grp7_servAddr, grp7_cltAddr;

    socklen_t grp7_cltAddr_len;

    int grp7_clients_track = 0;

    int opt = 1;

    grp7_servSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (grp7_servSocket == -1)

    {

        perror("Error: Failed to create socket");

        exit(1);
    }


    memset(&grp7_servAddr, 0, sizeof(grp7_servAddr));

    grp7_servAddr.sin_family = AF_INET;

    grp7_servAddr.sin_addr.s_addr = inet_addr(GRP7_SERV_IPADD); 

    grp7_servAddr.sin_port = htons(GRP7_SERVPORT); 

    if (bind(grp7_servSocket, (struct sockaddr *)&grp7_servAddr, sizeof(grp7_servAddr)) == -1)

    {

        perror("Error: Failed to bind socket");

        exit(1);
    }

    if (listen(grp7_servSocket, GRP7_MX_CONN) == -1)

    {

        perror("Error: Failed to listen");

        exit(1);
    }

    printf("Server waiting for client...\n");

    while (1)
    {

        grp7_cltAddr_len = sizeof(grp7_cltAddr);

        grp7_cltsockt = accept(grp7_servSocket, (struct sockaddr *)&grp7_cltAddr, &grp7_cltAddr_len);

        if (grp7_cltsockt == -1)

        {

            perror("Error: Failed to accept connection");

            exit(1);
        }

        printf("\nClient connected: %s:%d\n", inet_ntoa(grp7_cltAddr.sin_addr), ntohs(grp7_cltAddr.sin_port));


        if (grp7_clients_track < 6 || (grp7_clients_track >= 12 && (grp7_clients_track+1) % 2 == 1))

        {

            char welcome_message[] = "Welcome to the server!";
            if (send(grp7_cltsockt, welcome_message, strlen(welcome_message), 0) == -1)
            {
                perror("Error: Failed to send welcome message to client");
                exit(EXIT_FAILURE);
            }

            pid_t grp7_procID = fork();

            if (grp7_procID == -1)

            {

                close(grp7_cltsockt);

                perror("Error: fork failure");

                continue;
            }

            else if (grp7_procID == 0)

            {

                printf("Client %d handled by the server\n", grp7_clients_track + 1);

                close(grp7_servSocket); 

                processclient(grp7_cltsockt);

                exit(EXIT_SUCCESS);
            }

            else

            {

                grp7_clients_track++; 

                close(grp7_cltsockt); 

            }
        }
        else
        {
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

                printf("Client %d handled by the mirror\n", grp7_clients_track + 1);

                close(grp7_servSocket); 

                exit(EXIT_SUCCESS);
            }

            else

            {

                grp7_clients_track++; 

                close(grp7_cltsockt);

                printf("Mirror server(server.c) closed client connection from %s:%d\n", inet_ntoa(grp7_cltAddr.sin_addr), ntohs(grp7_cltAddr.sin_port));
            }
        }
    }

    return 0;
}