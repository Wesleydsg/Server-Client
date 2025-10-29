#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>

#define BUFFER_SIZE 4096
#define PORT 80

int main(int argc, char *argv[]){
    const char *hostname = argv[1];
    struct hostent *server;
    struct sockaddr_in server_addr;
    int sockfd;
    char buffer[BUFFER_SIZE];

    if(argc != 2){
        fprintf(stderr, "Error: Only one hostname argument is accepted for '%s <hostname>'\n", argv[0]);
        return 1;
    }

    // Resolving hostname
    if((server = gethostbyname(hostname)) == NULL){
        fprintf(stderr, "Error: This host does not exist\n");
        return 1;
    }

    // Creating socket
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        perror("Error creating the socket");
        return 1;
    }

    // Preparing server adress
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    memcpy(&server_addr.sin_addr.s_addr, server->h_addr_list[0], server->h_length);

    // Connecting to server
    if(connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0){
        perror("Error connecting to server");
        close(sockfd);
        return 1;
    }

    // Sende http GET request
    snprintf(buffer, sizeof(buffer), "GET / HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n", hostname);
    if(send(sockfd, buffer, strlen(buffer), 0) < 0){
        perror("Error sending request");
        close(sockfd);
        return 1;
    }

    // Recive and print response
    int recived;
    while((recived = recv(sockfd, buffer, sizeof(buffer)-1, 0)) > 0){
        buffer[recived] = '\0';
        printf("%s", buffer);
    }
    if(recived < 0){
        perror("Error reciving response");
    }

    // Closing socket
    close(sockfd);
    return 0;
}
