#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 4096

int main(){
    int server, new_socket;
    struct sockaddr_in address;
    int addreslen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};
    const char *response =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/plain\r\n"
        "Content-Length: 14\r\n"
        "\r\n"
        "Hello, World!\n";

    // Creating socket
    if((server = socket(AF_INET, SOCK_STREAM, 0)) == 0){
        perror("Error creating the socket");
        exit(EXIT_FAILURE);
    }

    // Server address config
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Socket connection to address and port
    if(bind(server, (struct sockaddr *)&address, sizeof(address)) < 0){
        perror("Error associating socket");
        close(server);
        exit(EXIT_FAILURE);
    }

    // Putting socket in listeaning mode
    if(listen(server, 3) < 0){
        perror("Listeaning error");
        close(server);
        exit(EXIT_FAILURE);
    }

    printf("HTTP server started at port %d...\n", PORT);

    // Loop for connection acceptance
    while(1){
        if((new_socket = accept(server, (struct sockaddr *)&address, (socklen_t *)&addreslen)) < 0){
            perror("Error accepting connection");
            continue;
        }

        //Reading client solicitation (optional)
        read(new_socket, buffer, BUFFER_SIZE);
        printf("Request recived:\n%s\n", buffer);

        // Sending HTTP anwser
        write(new_socket, response, strlen(response));

        // Closing client socket
        close(new_socket);
    }

    // Closing server socket
    close(server);

    return 0;
}