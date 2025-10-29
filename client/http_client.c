#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/stat.h>

#define BUFFER_SIZE 4096
#define PORT 8080

// Ensures the local download folder exists
void downloadFolder(){
    struct stat st = {0};
    if(stat("arquivos", &st) == -1){
        mkdir("arquivos", 0700);
    }
}

// Reads data until the end of HTTP header
int httpHeader(int sockfd, char *header, int max_size){
    int total = 0;
    while(total < max_size - 1){
        int n = recv(sockfd, header + total, 1, 0);
        if(n <= 0) return -1;
        total += n;
        header[total] = '\0';
        if(strstr(header, "\r\n\r\n")) break;
    }
    return total;
}

int main(int argc, char *argv[]){
    if(argc != 3){
        fprintf(stderr, "Usage: %s <hostname> <remote_file>\n", argv[0]);
        return 1;
    }

    const char *hostname = argv[1];
    const char *file_path = argv[2];

    struct hostent *server;
    struct sockaddr_in server_addr;
    int sockfd;
    char buffer[BUFFER_SIZE];

    // Resolving hostname
    if((server = gethostbyname(hostname)) == NULL){
        fprintf(stderr, "Error: host not found\n");
        return 1;
    }

    // Creating socket
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        perror("Error creating socket");
        return 1;
    }

    // Configuring server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    memcpy(&server_addr.sin_addr.s_addr, server->h_addr_list[0], server->h_length);

    // Connecting to the server
    if(connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0){
        perror("Error connecting to server");
        close(sockfd);
        return 1;
    }

    // Sending HTTP GET request
    snprintf(buffer, sizeof(buffer),
             "GET %s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n",
             file_path, hostname);
    send(sockfd, buffer, strlen(buffer), 0);

    // Reading HTTP header
    char header[8192];
    int header_len = httpHeader(sockfd, header, sizeof(header));
    if(header_len <= 0){
        fprintf(stderr, "Error reading HTTP header\n");
        close(sockfd);
        return 1;
    }

    printf("=== Received Header ===\n%s\n", header);

    // Checking if file exists on the server
    if(strstr(header, "404 Not Found")){
        fprintf(stderr, "Error 404: file not found on server.\n");
        close(sockfd);
        return 1;
    }

    // Extracting Content-Length value
    size_t content_length = 0;
    char *len_ptr = strcasestr(header, "Content-Length:");
    if(len_ptr){
        sscanf(len_ptr, "Content-Length: %zu", &content_length);
    }

    // Preparing local output file
    downloadFolder();
    const char *filename = strrchr(file_path, '/');
    filename = filename ? filename + 1 : file_path;

    char out_path[512];
    snprintf(out_path, sizeof(out_path), "arquivos/%s", filename);

    FILE *out = fopen(out_path, "wb");
    if(!out){
        perror("Error creating local file");
        close(sockfd);
        return 1;
    }

    // Write body bytes that arrived together with the header
    char *body_start = strstr(header, "\r\n\r\n");
    size_t body_bytes = 0;
    if(body_start){
        body_start += 4;
        body_bytes = header_len - (body_start - header);
        if(body_bytes > 0)
            fwrite(body_start, 1, body_bytes, out);
    }

    // Continue reading the remaining body
    size_t total_received = body_bytes;
    int n;
    while((n = recv(sockfd, buffer, sizeof(buffer), 0)) > 0){
        fwrite(buffer, 1, n, out);
        total_received += n;

        if(content_length > 0 && total_received >= content_length)
            break;
    }

    fclose(out);
    close(sockfd);

    printf("File '%s' downloaded successfully! (%zu bytes received)\n", out_path, total_received);
    return 0;
}
