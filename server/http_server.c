#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <time.h>
#include <signal.h>

#define PORT 8080
#define BUFFER_SIZE 4096
#define ROOT_DIR "./arquivos"

// https://andy2903-alp.medium.com/criando-um-servidor-http-simples-em-c-8fcaf5d794c3

const char* getMimeType(const char* path){
    if(strstr(path, ".html")) return "text/html";
    if(strstr(path, ".css")) return "text/css";
    if(strstr(path, ".png")) return "image/png";
    if(strstr(path, ".jpg") || strstr(path, "jpeg")) return "image/jpeg";
    if(strstr(path, ".gif")) return "image/gif";
    if(strstr(path, ".mp4")) return "video/mp4";
    if(strstr(path, ".webm")) return "audio/webm";
    return "text/plain";
}

const char* getFolderType(const char* path){
    if (strstr(path, ".html") || strstr(path, ".css")) return "codigos";
    if (strstr(path, ".png") || strstr(path, ".jpg") || strstr(path, ".jpeg") || strstr(path, ".gif")) return "imagens";
    if (strstr(path, ".mp4")) return "videos";
    if (strstr(path, ".webm")) return "musicas";
    return "textos";
}

void sendFile(int client, const char* path){
    FILE* file = fopen(path, "rb");
    // Not an existing file
    if(!file){
        const char* not_found =
            "HTTP/1.1 404 Not Found\r\n"
            "Content-Type: text/plain\r\n"
            "Content-Length: 9\r\n"
            "\r\n"
            "Not Found";
        write(client, not_found, strlen(not_found));
        return;
    }

    struct stat st;
    stat(path, &st);
    
    size_t file_size = st.st_size;

    const char* mime = getMimeType(path);
    char header[256];
    snprintf(header, sizeof(header),
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: %s\r\n"
            "Content-Length: %zu\r\n"
            "\r\n", mime, file_size);
    write(client, header, strlen(header));

    char file_buffer[BUFFER_SIZE];
    size_t bytes;
    while((bytes = fread(file_buffer, 1, sizeof(file_buffer), file)) > 0){
        write(client, file_buffer, bytes);
    }

    fclose(file);
}

int main(){
    int server, client;
    struct sockaddr_in address;
    int addreslen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};
    signal(SIGPIPE, SIG_IGN); // Enabled other machines to connect and grab videos from server

    // Creating socket
    if((server = socket(AF_INET, SOCK_STREAM, 0)) == 0){
        perror("Error creating the socket");
        exit(EXIT_FAILURE);
    }
    
    int opt = 1;
    setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
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

    // Putting socket in listeaning mode (10 resquests now)
    if(listen(server, 10) < 0){
        perror("Listeaning error");
        close(server);
        exit(EXIT_FAILURE);
    }

    printf("HTTP server started at port %d...\n", PORT);

    // Loop for connection acceptance
    while(1){
        client = accept(server, (struct sockaddr *)&address, (socklen_t *)&addreslen);
        if(client < 0){
            perror("Error accepting connection");
            continue;
        }

        //Reading client solicitation
        memset(buffer, 0, sizeof(buffer));
        read(client, buffer, sizeof(buffer) - 1);
        printf("Request recived:\n%s\n", buffer);

        // Request path and method
        char method[8], path[256];
        sscanf(buffer, "%s %s", method, path);

        if(strcmp(method, "GET") != 0){
            const char* error = "HTTP/1.1 205 Method Not Allowed\r\n\r\n";
            write(client, error, strlen(error));
            close(client);
            continue;
        }

        if(strcmp(path, "/") == 0)
            strcpy(path, "/Kermit.gif");

        // Full path summurize
        const char* subfolder = getFolderType(path);
        char full_path[512];
        snprintf(full_path, sizeof(full_path), "%s/%s%s", ROOT_DIR, subfolder, path);

        // Sending HTTP anwser
        sendFile(client, full_path);

        // Closing client socket
        close(client);
    }

    // Closing server socket
    close(server);

    return 0;
}