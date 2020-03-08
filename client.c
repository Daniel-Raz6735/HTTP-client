/*Daniel Raz*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <getopt.h>
#include <errno.h>
#include <netdb.h>

//In any case of wrong command usage, print this messege:
#define usegeError printf("Usage: client [-p <text>] [-r n < pr1=value1 pr2=value2 …>] <URL>\n");
#define SIZE_BUFFER 100

/*---------------private function-----------------------*/
void checkRParam(char **argv, int n, int index, int argc);
int validNumOfR(char *n);
int indexOf(char *str, char c);
/*------------------------------------------------------*/

int main(int argc, char **argv) 
{

    /*--------------------set of variables------------------------------*/
    char ht[] = "http://", *request, *filePath = NULL, *host, content[30], buffer[SIZE_BUFFER];
    int numOfArg = -1, rIndex = 0, pIndex = 0, htIndex = 0, len = 50, sum = 0;
    int hostIndex = 0, portIndex = 0, conetentLength, port;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    /*-------------------------------------------------------------------*/

    if (argc < 2)
    {
        usegeError;
        exit(EXIT_FAILURE);
    }

    // this for check if the input is valid.
    for (int i = 1; i < argc;)
    {
        //look for -p flag.
        if (strcmp(argv[i], "-p") == 0)
        {
            pIndex = i;
            conetentLength = strlen(argv[i + 1]);
            len += 19 + conetentLength; //for the size of the request 19 =length("content-length" +"POST").

            if (i + 2 > argc)
            {
                usegeError;
                exit(EXIT_FAILURE);
            }
            i += 2;
        }
        //look for -r flag.
        if (i < argc && strcmp(argv[i], "-r") == 0)
        {
            rIndex = i;
            if (validNumOfR(argv[i + 1]) != -1)
                numOfArg = atoi(argv[i + 1]);
            else
            {
                usegeError;
                exit(EXIT_FAILURE);
            }

            if (numOfArg != 0)
            {
                checkRParam(argv, numOfArg, rIndex + 2, argc);
                int index = rIndex + 2;
                for (int j = 0; j < numOfArg; j++)
                {
                    len += strlen(argv[index]) + numOfArg;
                    index++;
                }
            }
            i += (2 + numOfArg);
            continue;
        }
        //look for http://.....
        if (i < argc)
            for (int j = 0; j < 7; j++)
            {
                if (argv[i][j] != ht[j])
                {
                    usegeError;
                    exit(EXIT_FAILURE);
                }
                htIndex = i;
            }
        i++;
    }

    //if the request without url usege error.
    if (htIndex == 0)
    {
        usegeError;
        exit(EXIT_FAILURE);
    }

    portIndex = indexOf(argv[htIndex], ':');
    if (portIndex != -1) 
    {//if port was found at url and he valid take the host and the relevent port.

        hostIndex = indexOf(argv[htIndex], '/');
        if (hostIndex != -1 && hostIndex > portIndex)
        {
            char *strPort = (char *)malloc(sizeof(char) * (hostIndex - portIndex));
            if (strPort == NULL)
            {
                perror("malloc:");
                exit(EXIT_FAILURE);
            }

            int i = portIndex + 1;
            for (int j = 0; j < hostIndex - portIndex; j++)
            {
                strPort[j] = argv[htIndex][i];
                i++;
            }
            port = atoi(strPort);
            if (port <= 0)
            {
                usegeError;
                free(strPort);
                exit(EXIT_FAILURE);
            }
            free(strPort);
        }
        else
        {
            port = 80;
        }

        host = (char *)malloc(sizeof(char) * (portIndex));
        if (host == NULL)
        {
            perror("malloc:");
            exit(EXIT_FAILURE);
        }

        strncpy(host, argv[htIndex] + 7, portIndex - 7);
        host[portIndex - 7] = '\0';
    }
    else
    {//take the host part with default port:80.

        port = 80;
        hostIndex = indexOf(argv[htIndex], '/');

        if (hostIndex == -1)
        {
            filePath = "/";
            host = strchr(argv[htIndex], 'w');
            if (host == NULL)
                host = "";
        }
        else
        {
            host = (char *)malloc(sizeof(char) * (hostIndex - 6));
            if (host == NULL)
            {
                perror("malloc:");
                exit(EXIT_FAILURE);
            }
            host[hostIndex - 7] = '\0';
            strncpy(host, argv[htIndex] + 7, hostIndex - 7);
            filePath = strchr(argv[htIndex] + 7, '/');
            if (filePath == NULL)
                filePath = "/";
        }
    }

    if (filePath == NULL)
        filePath = strchr(argv[htIndex] + 7, '/');
    if (filePath == NULL)
        filePath = "/";

    /*----------------------make the request---------------------*/
    len += strlen(host) + strlen(filePath);
    request = (char *)malloc(sizeof(char) * len);
    if (request == NULL)
    {
        if (strcmp(filePath, "/") != 0)
            free(host);
        perror("malloc:");
        exit(EXIT_FAILURE);
    }
    request[0] = '\0';
    if (pIndex != 0)
        strcat(request, "POST ");
    else
        strcat(request, "GET ");

    if (rIndex != 0)
    {
        strcat(request, filePath);
        strcat(request, "?");
        int index = rIndex + 2;

        for (int i = 0; i < numOfArg; i++)
        {
            strcat(request, argv[index]);
            if (i < numOfArg - 1)
                strcat(request, "&");
            index++;
        }
    }
    else
    {
        strcat(request, filePath);
    }

    strcat(request, " HTTP/1.0\r\n");
    strcat(request, "HOST: ");
    strcat(request, host);
    strcat(request, "\r\n");
    if (pIndex != 0)
    {
        sprintf(content, "Content-length:%ld\r\n\r\n", strlen(argv[pIndex + 1]));
        strcat(request, content);
        strcat(request, argv[pIndex + 1]);
    }
    else
    {
        strcat(request, "\r\n");
    }
    /*---------------------------------------------------------*/

    //create the socket.
    int sd = socket(AF_INET, SOCK_STREAM, 0);
    if (sd < 0)
    {
        if (strcmp(filePath, "/") != 0)
            free(host);
        free(request);
        perror("socket:");
        exit(EXIT_FAILURE);
    }

    // initlaize the struct with the ip and port.
    server = gethostbyname(host);
    if (server == NULL)
    {
        if (strcmp(filePath, "/") != 0)
            free(host);
        free(request);
        herror("gethostbyname:");
        exit(EXIT_FAILURE);
    }

    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr_list[0], (char *)&serv_addr.sin_addr, server->h_length);
    serv_addr.sin_port = htons(port);
    //-----------------------------------------------------------------------------------

    //try connect to the socket.
    int rc = connect(sd, (const struct sockaddr *)&serv_addr, sizeof(serv_addr));
    if (rc < 0)
    {
        if (strcmp(filePath, "/") != 0)
            free(host);
        free(request);
        perror("connect:");
        exit(EXIT_FAILURE);
    }
    //print the request before the send.
    printf("HTTP request =\n%s\nLEN = %ld\n", request, strlen(request));

    int send = write(sd, request, strlen(request)+1);
    if (send < 0)
    {
        if (strcmp(filePath, "/") != 0)
            free(host);
        free(request);
        perror("write:");
        exit(EXIT_FAILURE);
    }

    int r;
    while ((r = read(sd, buffer, SIZE_BUFFER)) != 0) //read the response from the server to buffer[].
    {
        if (r < 0)
        {
            if (strcmp(filePath, "/") != 0)
                free(host);
            free(request);
            perror("read:");
            exit(EXIT_FAILURE);
        }
        buffer[r] = '\0';
        printf("%s", buffer);
        sum += r;
    }
    printf("\n Total received response bytes: %d\n", sum);

    if (strcmp(filePath, "/") != 0)
        free(host);
    free(request);
    close(sd);

    return 0;
}

//this function for check if the arguments after -r flag are valid.
void checkRParam(char **argv, int n, int index, int argc)
{
    char *check;

    if (n + index > argc)
    {
        usegeError;
        exit(EXIT_FAILURE);
    }

    for (int j = index, i = 0; i < n; i++)
    {
        check = strchr(argv[j], '=');
        if (check == NULL)
        {
            usegeError;
            exit(EXIT_FAILURE);
        }
        else if (argv[j][0] == '=' || strlen(check) == 1)
        {
            usegeError;
            exit(EXIT_FAILURE);
        }
        j++;
    }
}

//this function check if a number shows after -r flag, return -1 if not.
int validNumOfR(char *n)
{
    int x = atoi(n);

    if (x != 0)
        return x;

    if (x == 0 && strcmp(n, "0") == 0)
        return 0;

    return -1;
}

//return the index where the char 'c' showing.
int indexOf(char *str, char c)
{
    for (int i = 7; i < strlen(str); i++)
    {
        if (str[i] == c)
            return i;
    }
    return -1;
}
