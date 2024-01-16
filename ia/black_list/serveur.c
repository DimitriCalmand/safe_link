#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>

#define BUF_SIZE 20

void quit_client(int sfd)
{
    const char *message = "exit";
    if (write(sfd, message, strlen(message)) != (ssize_t) strlen(message)) {
        fprintf(stderr, "Failed to send message\n");
        exit(EXIT_FAILURE);
    }
}
void update_file(const char *filename, char *message)
{
    FILE *f = fopen(filename, "w");
    if (f == NULL)
    {
        printf("Error opening file!\n");
        return;
    }

    fprintf(f, "%s", message);
    fclose(f);
}
char *read_file(const char *filename)
{
    char *buffers = NULL;
    long length;
    FILE *f = fopen(filename, "rb");

    if (f)
    {
        fseek(f, 0, SEEK_END);
        length = ftell(f);
        fseek(f, 0, SEEK_SET);
        buffers = malloc(length + 1);  // Add 1 for the null character
        if (buffers)
        {
            int a = fread(buffers, 1, length, f);
            a++;
            buffers[length] = '\0';  // Add the null character at the end
        }
        fclose(f);
    }

    return buffers;
}


int main2(int argc, char *argv[])
{
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    int sfd, s;
    struct sockaddr_storage peer_addr;
    socklen_t peer_addr_len;
    ssize_t nread;
    char buf[BUF_SIZE];

    if (argc != 2) {
        fprintf(stderr, "Usage: %s port\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    s = getaddrinfo(NULL, argv[1], &hints, &result);
    if (s != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        exit(EXIT_FAILURE);
    }

    for (rp = result; rp != NULL; rp = rp->ai_next) {
        sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sfd == -1)
            continue;

        if (bind(sfd, rp->ai_addr, rp->ai_addrlen) == 0)
            break;

        close(sfd);
    }

    if (rp == NULL) {
        fprintf(stderr, "Could not bind\n");
        exit(EXIT_FAILURE);
    }

    freeaddrinfo(result);

    if (listen(sfd, SOMAXCONN) == -1) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    char *message = read_file("list.txt");
    if (message == NULL) 
    {
        fprintf(stderr, "Failed to read file\n");
        exit(EXIT_FAILURE);
    }
    ssize_t len = strlen(message);  



    for (;;) {

        peer_addr_len = sizeof(peer_addr);
        int cfd = accept(sfd, (struct sockaddr *) &peer_addr, &peer_addr_len);

        if (cfd == -1) {
            perror("accept");
            continue;
        }
        if (write(cfd, message, len) != len) 
        {
            fprintf(stderr, "Failed to send message\n");
            exit(EXIT_FAILURE);
        }
        int receive = 0;
        while ((nread = read(cfd, buf, BUF_SIZE-1)) > 0)
        {      
            message = realloc(message, strlen(message) + strlen(buf) + 1);
            strcat(message, buf);                
            memset(buf, 0, BUF_SIZE);
            receive = 1;
            if (nread != BUF_SIZE-1)
            {
                break;
            }
        }
        if (receive)
        {
            len = strlen(message);
            update_file("list.txt", message);
            char* verif = "receive message";
            int len_verif = strlen(verif);
            if (write(cfd, verif, len_verif) != len_verif) 
            {
                fprintf(stderr, "Failed to send message\n");
                exit(EXIT_FAILURE);
            }
        }

        
        if (nread == -1)
        {
            perror("\nerror read");
            close(cfd);
            continue;
        }

        if (close(cfd) == -1) {
            perror("close");
            exit(EXIT_FAILURE);
        }
    }
}