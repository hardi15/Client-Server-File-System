// Jeel Jadawala - 110096762 | Hardi Kadia -

// Section 1

// client.c

#include <stdio.h>

#include <stdlib.h>

#include <string.h>

#include <unistd.h>

#include <arpa/inet.h>

#include <fcntl.h>

#include <time.h>

#include <signal.h>

// defining global constants

#define GRP7_SERV_IPADD "127.0.0.1" // Server IP

#define GRP7_SERV_PORT 3490 //  server Port

#define GRP7_BUFSIZ 10000 // buffer to send data to server

#define GRP7_MX_PATHLEN 4096 // max size for command string

#define GRP7_MX_ARG 6 // max number of args that can be passed by client

// function to unzip the tar file

void uzip_tarfl(char *grp7_tar_flname)

{

    printf("unzipping in progress....\n");

    char grp7_cmmd[GRP7_MX_PATHLEN + 10];

    // extracting the tar file content to the current working directory

    sprintf(grp7_cmmd, "tar -xzf %s -C %s", grp7_tar_flname, getenv("PWD"));

    int grp7_stats = system(grp7_cmmd);

    // error condition

    if (grp7_stats == -1)

    {

        perror("unzipping failed!");

        exit(1);
    }

    printf("File successfully unzipped!\n");
}

// function to receive tar file from server/mirror

void get_tar_file(int grp7_sockgrp7_fldp, int grp7_isgrp7_uzip)
{
    // Fork a new process
    int grp7_procID = fork();

    // Child process
    if (grp7_procID == 0)
    {
        // Declare variables
        char grp7_bufr[GRP7_BUFSIZ]; // Buffer to store received data
        char grp7_tarflname[40];    // Name of the temporary tar file
        sprintf(grp7_tarflname, "temp.tar.gz"); // Set the temporary file name

        // Open a file for writing
        int grp7_fldp = open(grp7_tarflname, O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);
        if (grp7_fldp < 0)
        {
            perror("error in opening file descriptor");
            exit(EXIT_FAILURE);
        }

        // Receive and write file contents byte by byte
        ssize_t grp7_bytesrecv = 1;   // Bytes received in each iteration
        long int grp7_total_bytesrecv = 0; // Total bytes received
        while (grp7_bytesrecv > 0)
        {
            // Clear the buffer
            memset(grp7_bufr, 0, sizeof(grp7_bufr));

            // Receive data from the server into the buffer
            grp7_bytesrecv = recv(grp7_sockgrp7_fldp, grp7_bufr, GRP7_BUFSIZ, 0);

            // Check for receive error
            if (grp7_bytesrecv < 0)
            {
                perror("recv fail");
                exit(EXIT_FAILURE);
            }

            // Write received data to the local file
            long int grp7_byteswrit = write(grp7_fldp, grp7_bufr, grp7_bytesrecv);

            // Check for write error
            if (grp7_byteswrit < 0)
            {
                perror("write fail");
                exit(EXIT_FAILURE);
            }

            // Clear the buffer for the next iteration
            memset(grp7_bufr, 0, sizeof(grp7_bufr));

            // Update the total bytes received
            grp7_total_bytesrecv += grp7_byteswrit;
        }

        // Check if no data received
        if (grp7_total_bytesrecv == 0)
        {
            printf("File not received.\n");
        }
        else
        {
            // Print success message and the total bytes received
            printf("File received successfully.\n");
            printf("Total file received %d\n", grp7_total_bytesrecv);
        }

        // Close the file descriptor and exit the child process
        close(grp7_fldp);
        exit(0);
    }
    else
    {
        // Parent process

        // Wait for 10 seconds
        sleep(10);

        // Kill the child process
        kill(grp7_procID, SIGKILL);

        // If flag is set to 1, unzip the tar file
        if (grp7_isgrp7_uzip == 1)
        {
            uzip_tarfl("temp.tar.gz");
        }
    }
}


int main()

{

    int grp7_cltsock;

    struct sockaddr_in grp7_servAddr, grp7_mirAddr;

    char grp7_bufr[GRP7_BUFSIZ];

    ssize_t grp7_bytesrecv;

    int file_grp7_fldp;

    // Create a socket

    grp7_cltsock = socket(AF_INET, SOCK_STREAM, 0);

    if (grp7_cltsock == -1)

    {

        perror("Error: Socket Creation failure");

        exit(1);
    }

    memset(&grp7_servAddr, 0, sizeof(grp7_servAddr));

    // server address configuration

    grp7_servAddr.sin_family = AF_INET;

    grp7_servAddr.sin_port = htons(GRP7_SERV_PORT);

    if (inet_pton(AF_INET, GRP7_SERV_IPADD, &grp7_servAddr.sin_addr) == -1)

    {

        perror("inet_pton");

        exit(EXIT_FAILURE);
    }

    // Server connection

    if (connect(grp7_cltsock, (struct sockaddr *)&grp7_servAddr, sizeof(grp7_servAddr)) == -1)

    {

        perror("Error: Server connection failure");

        exit(1);
    }

    // printf("\nServer connection at : %s:%d\n", GRP7_SERV_IPADD, GRP7_SERV_PORT);

    char grp7_cmmd[1000], grp7_srvmsg[2000];

    char *grp7_arguments[GRP7_MX_ARG];

    int grp7_argCount;

    // continuous loop for client server communication

    char grp7_mirDetails[GRP7_BUFSIZ];

    ssize_t ip_port_received = recv(grp7_cltsock, grp7_mirDetails, GRP7_BUFSIZ, 0);

    printf("%s", grp7_mirDetails);

    // condition to check if server is loaded, pass the connection to mirror

    if (strcmp(grp7_mirDetails, "Welcome to the mirror!") == 0)

    {

        close(grp7_cltsock); // Close the unnecessary connection to the server

        // Create a new socket for mirror server

        int grp7_mirsockt = socket(AF_INET, SOCK_STREAM, 0);

        memset(&grp7_mirAddr, 0, sizeof(grp7_mirAddr));

        grp7_mirAddr.sin_family = AF_INET;

        grp7_mirAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

        grp7_mirAddr.sin_port = htons(8081);

        if (connect(grp7_mirsockt, (struct sockaddr *)&grp7_mirAddr, sizeof(grp7_mirAddr)) == -1)

        {

            perror("Error: Oops! Connection to mirror failed");

            exit(1);
        }

        printf("Yayy! Connected to the mirror.\n");
    }

    while (1)

    {

        // User prompt

        printf("\nC$: ");

        fgets(grp7_cmmd, 1000, stdin);

        char temp[1000];

        sprintf(temp, "%s", grp7_cmmd);

        grp7_argCount = 0;

        grp7_arguments[grp7_argCount] = strtok(temp, " \n");

        // parse user input into arguments

        while (grp7_arguments[grp7_argCount] != NULL)

        {

            grp7_argCount++;

            grp7_arguments[grp7_argCount] = strtok(NULL, " \n");
        }

        // initialize required flags

        int grp7_cmmd_valid_flag = 0;

        int grp7_flflag = 0;

        int grp7_uzip = 0;

        // compare the argument input and match with required command to check its valid or not

        if (strcmp(grp7_arguments[0], "filesrch") == 0)

        {

            grp7_flflag = 0;

            if (grp7_argCount != 2)

            {

                fprintf(stderr, "Usage: %s filename\n", grp7_arguments[0]);
            }

            else

            {

                grp7_cmmd_valid_flag = 1;
            }
        }

        else if (strcmp(grp7_arguments[0], "tarfgetz") == 0)

        {

            grp7_cmmd_valid_flag = 1;

            if (grp7_argCount < 3 || grp7_argCount > 4)

            {

                fprintf("Usage: %s size1 size2 [-u]\n", grp7_arguments[0]);

                grp7_cmmd_valid_flag = 0;
            }

            else

            {

                long grp7_firstsize = atol(grp7_arguments[1]);

                long grp7_secondsize = atol(grp7_arguments[2]);

                if (grp7_firstsize < 0 || grp7_secondsize < 0 || grp7_firstsize > grp7_secondsize)

                {

                    printf("Sorry! Provided size params are invalid\n");

                    grp7_cmmd_valid_flag = 0;
                }
            }

            if (grp7_cmmd_valid_flag)

            {

                grp7_flflag = 1;

                if (strcmp(grp7_arguments[grp7_argCount - 1], "-u") == 0)

                {

                    grp7_uzip = 1;
                }
            }
        }

        else if (strcmp(grp7_arguments[0], "getdirf") == 0)

        {

            grp7_cmmd_valid_flag = 1;

            if (grp7_argCount < 3 || grp7_argCount > 4)

            {

                printf("Usage: %s date1 date2 [-u]\n", grp7_arguments[0]);

                grp7_cmmd_valid_flag = 0;
            }

            else

            {

                struct tm grp7_firstdate_tm, grp7_seconddate_tm;

                memset(&grp7_firstdate_tm, 0, sizeof(grp7_firstdate_tm));

                memset(&grp7_seconddate_tm, 0, sizeof(grp7_seconddate_tm));

                if (strptime(grp7_arguments[1], "%Y-%m-%d", &grp7_firstdate_tm) == NULL || strptime(grp7_arguments[2], "%Y-%m-%d", &grp7_seconddate_tm) == NULL)

                {

                    printf("Invalid date format: YYYY-MM-DD\n");

                    grp7_cmmd_valid_flag = 0;
                }

                time_t grp7_firstdate = mktime(&grp7_firstdate_tm);

                time_t grp7_seconddate = mktime(&grp7_seconddate_tm);

                if (grp7_firstdate == -1 || grp7_seconddate == -1 || grp7_firstdate > grp7_seconddate)

                {

                    printf("Sorry! Provided date parameters are invalid\n");

                    grp7_cmmd_valid_flag = 0;
                }
            }

            if (grp7_cmmd_valid_flag)

            {

                grp7_flflag = 1;

                if (strcmp(grp7_arguments[grp7_argCount - 1], "-u") == 0)

                {

                    grp7_uzip = 1;
                }
            }
        }

        else if (strcmp(grp7_arguments[0], "fgets") == 0)

        {

            //printf("%d", grp7_argCount);

            if (grp7_argCount < 2 || grp7_argCount > 5)

            {

                printf("Usage: %s file1 [file2 ... file4]\n", grp7_arguments[0]);
            }

            else

            {

                grp7_flflag = 1;

                grp7_cmmd_valid_flag = 1;
            }
        }

        else if (strcmp(grp7_arguments[0], "targzf") == 0)

        {

            int GRP7_MX_ARG_TARGZF = 5; // Maximum allowed arguments without -u

            if (grp7_argCount < 2 || (grp7_argCount == 2 && strcmp(grp7_arguments[1], "-u") == 0) || grp7_argCount > GRP7_MX_ARG_TARGZF + 1 || (grp7_argCount == GRP7_MX_ARG_TARGZF + 1 && strcmp(grp7_arguments[grp7_argCount - 1], "-u") != 0))

            {

                printf("Usage: %s extension1 [extension2 ... extension4] [-u]\n", grp7_arguments[0]);
            }

            else

            {

                grp7_cmmd_valid_flag = 1;

                grp7_flflag = 1;

                if (strcmp(grp7_arguments[grp7_argCount - 1], "-u") == 0)

                {

                    grp7_uzip = 1;
                }
            }
        }

        else if (strcmp(grp7_arguments[0], "quit") == 0)

        {

            printf("You chose to quit...\n");

            break;
        }

        else

        {

            printf("Bad Request! command not supported by server\n");
        }

        if (grp7_cmmd_valid_flag)

        {

            write(grp7_cltsock, grp7_cmmd, strlen(grp7_cmmd));

            if (grp7_flflag)

            {

                printf("file being received...\n");

                get_tar_file(grp7_cltsock, grp7_uzip);
            }

            else

            {

                // Receive message from server

                memset(grp7_srvmsg, 0, sizeof(grp7_srvmsg));

                if (recv(grp7_cltsock, grp7_srvmsg, 2000, 0) < 0)

                {

                    printf("Failure in receiving message.");

                    break;
                }

                printf("%s", grp7_srvmsg);

                memset(grp7_srvmsg, 0, sizeof(grp7_srvmsg));
            }
        }
    }

    close(grp7_cltsock);

    return 0;
}