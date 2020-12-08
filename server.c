#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

void error(const char *msg) {
    perror(msg);
    exit(1);
}

int getNickname(int sock, char *buffer) {
    int n;
    
    bzero(buffer, 10);
    n = read(sock, buffer, 9);
    if (n == 0) {
        printf("Client has been disconnected\n");
        
        return 0;
    }
    
    if (n < 0)
        error("ERROR reading from socket");
    printf("%s has been connected to the server\n", buffer);
    
    return 1;
}

int dostuff(int sock, const char *nickname) {
    int n;
    char buffer[256];
    
    bzero(buffer, 256);
    n = read(sock, buffer, 255);
    if (n == 0) {
        printf("%s has been disconnected\n", nickname);
        
        return 0;
    }
    
    if (n < 0)
        error("ERROR reading from socket");
    printf("%s: %s", nickname, buffer);
    
    return 1;
}

int main(int argc, char *argv[]) {
    int sockfd, newsockfd, portno, flag;
    pid_t pid;
    socklen_t clilen;
    char nickname[10];
    struct sockaddr_in serv_addr, cli_addr;
    
    if (argc < 2) {
        fprintf(stderr, "ERROR, no port provided\n");
        exit(1);
    }
    
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");
    
    bzero((char *) &serv_addr, sizeof(serv_addr));
    portno = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    
    if (bind(sockfd, (struct sockaddr *) &serv_addr,
             sizeof(serv_addr)) < 0)
        error("ERROR on binding");
    
    printf("Parent PID: %d\n", getpid());
    
    flag = 1;
    listen(sockfd, 5);
    clilen = sizeof(cli_addr);
    while (1) {
        newsockfd = accept(sockfd,
                           (struct sockaddr *) &cli_addr,
                           &clilen);
        if (newsockfd < 0)
            error("ERROR on accept");
        
        pid = fork();
        if (pid == 0) {
            close(sockfd);
            flag = getNickname(newsockfd, nickname);
            while(flag)
                flag = dostuff(newsockfd, nickname);
            exit(0);
        } else {
            close(newsockfd);
        }
    }
    
    close(sockfd);
    
    return 0;
}
