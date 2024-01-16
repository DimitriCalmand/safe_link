#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>

#define BUF_SIZE 200

void send_new_spamer(int sfd, char *spamer)
{
    ssize_t len = strlen(spamer);
    if (write(sfd, spamer, len) != len) 
    {
        fprintf(stderr, "Failed to send message\n");
        exit(EXIT_FAILURE);
    }

}
int connect_client(char* ip, char* port)
{
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    int sfd, s;



    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    s = getaddrinfo(ip, port, &hints, &result);
    if (s != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        exit(EXIT_FAILURE);
    }

    for (rp = result; rp != NULL; rp = rp->ai_next) {
        sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sfd == -1)
            continue;

        if (connect(sfd, rp->ai_addr, rp->ai_addrlen) != -1)
            break;

        close(sfd);
    }

    if (rp == NULL) {
        fprintf(stderr, "Could not connect\n");
        exit(EXIT_FAILURE);
    }

    freeaddrinfo(result);
    return sfd;
}
void send_spam(char* ip, char* port, char* mail)
{
    int sfd = connect_client(ip, port);
    sleep(1);
    send_new_spamer(sfd, mail);
    shutdown(sfd, SHUT_WR);
    char buf[BUF_SIZE];
    while (read(sfd, buf, BUF_SIZE) > 0)
    {
        //nothing
    }

    close(sfd);
}
char** get_email_spamer(char* list, int* size)
{
    *size = 0;
    char* split = strtok(list, "\n");
    char** res = NULL;
    while (split != NULL)
    {
        size[0]++;
        res = realloc(res, size[0] * sizeof(char*));
        res[size[0]-1] = split;
        split = strtok(NULL, "\n");
    }
    return res;
}

char* get_spam(char* ip, char* port)
{
    int sfd = connect_client(ip, port);
    ssize_t nread;
    char buf[BUF_SIZE];
    char* list = malloc(1);
    *list = '\0';
    int k = 0;
    memset(buf, 0, BUF_SIZE);
    while ((nread = read(sfd, buf, BUF_SIZE-1)) > 0) 
    {
        list = realloc(list, (BUF_SIZE-1)*(k+1)+1);
        strcat(list, buf);
        memset(buf, 0, BUF_SIZE);
        k++;
        if (nread != BUF_SIZE-1)
        {
            break;
        }
    }
    // char** res = get_email_spamer(list, size);    
    close(sfd);
    return list;
}




/*
int sfd = connect_client(argv[1], argv[2]);
    ssize_t nread;
    char buf[BUF_SIZE];
    
    while ((nread = read(sfd, buf, BUF_SIZE)) > 0) 
    {
        if (write(STDOUT_FILENO, buf, nread) != nread) 
        {
            fprintf(stderr, "Failed to write to stdout\n");
            exit(EXIT_FAILURE);
        }
        memset(buf, 0, BUF_SIZE);
    }
    sleep(2);
    send_new_spamer(sfd, "#dimitr.calmand@gmail.com");
    close(sfd);
    return 0;
*/