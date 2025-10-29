# Simple HTTP Server & Client in C

This project implements a **minimal HTTP/1.1 server and client in C**, built from scratch using raw TCP sockets.  
It allows you to **host and download static files** (texts, images, videos, etc.) over HTTP — no external libraries required.

---

## Project Overview

### `http_server.c`
A HTTP server that:
- Listens on port **8080**. (can be changed in code)
- Serves static files stored inside the `./server/arquivos` directory.
- Automatically detects MIME types (`.html`, `.css`, `.png`, `.jpg`, `.gif`, `.mp4`, `.webm` and `.txt` as the last option for any different types).
- Organizes files into subfolders based on type:
```
arquivos/
├── codigos/
├── imagens/
├── videos/
├── musicas/
└── textos/
```

- Returns HTTP responses (`200 OK`, `404 Not Found`).
- Works directly in any web browser (e.g., http://localhost:8080).

### `http_client.c`
A HTTP client that:
- Connects to an HTTP server via TCP on port **8080**. (can be changed in code)
- Sends a GET request for a remote file.
- Parses the HTTP header.
- Saves the downloaded file inside a local folder `./client/arquivos/`. That folder is created in the first client run
- Ensures data integrity (byte count matches server content length).

## Project Structure

```
server-client/
|
├── client/
|   ├── base_client.c
|   ├── http_client.c
|   └── Makefile
|
├── server
|   ├── arquivos/
|   ├──base_server.c
|   ├──http_server.c
|   └── Makefile
|
└── README.md

```

## How It Works

### 🔹 Server

1. Creates a socket and binds it to port **8080**.  
2. Waits for incoming HTTP `GET` requests.  
3. Determines which subfolder to read from using `get_folder_type()`.  
4. Reads and sends the requested file to the client, preceded by the appropriate HTTP header.  
5. Handles invalid or missing files with a **404 response**.

### 🔹 Client

1. Resolves the target hostname (e.g., `localhost` or `127.0.0.1`).  
2. Connects to the server on port **8080**.  
3. Sends an HTTP `GET` request for a specific file path (e.g., `/Kermit.gif`).  
4. Reads the HTTP header until `\r\n\r\n`.  
5. Extracts the `Content-Length` to determine file size.  
6. Writes all received bytes to `arquivos/<filename>`.  
7. Closes the connection once transfer is complete.

---

## Compilation

- Server
```bash
cd server
make
```

- Client

```bash
cd client
make
```

---

## How to Run
```bash
# Start the server
./server

# Run the client
./client http://host /<filename>
```
