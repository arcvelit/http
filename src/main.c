#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include <pthread.h>

#include <sys/socket.h>
#include <arpa/inet.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 6969
#define BUFFER_SIZE 1024
#define INCOMING_CONNECTIONS 10

#define ASSERT_MEM(PTR)                              \
    do {                                             \
        if (!(PTR)) {                                \
            fprintf(stderr, "FATAL: Out of memory"); \
            exit(EXIT_FAILURE);                      \
        }                                            \
    } while (0)

#define ASSERT_SUCCESS(val, message) \
    do {                             \
        if ((val) < 0) {             \
            perror(message);         \
            exit(1);                 \
        }                            \
    } while(0)

#define ASSERT_CLOSE_SOCK(val, sock, message) \
    do {                                      \
        if ((val < 0)) {                      \
            perror(message);                  \
            close((sock));                    \
            exit(1);                          \
        }                                     \
    } while (0)

typedef struct {
    // Address structure
    struct sockaddr_in addr;
    socklen_t addr_len;
    // Socket file descriptor
    int fd;
    // Simple ip and port
    int port;
    char* ip;
} Server;

typedef struct {
    char* items;
    size_t size;
    size_t cap;
} Response;


void response_alloc(Response* res) {
    res->items = malloc(BUFFER_SIZE);
    ASSERT_MEM(res->items);
    res->cap = BUFFER_SIZE;
    res->size = 0;
}

void response_append_buf(Response* res, const char* buf, size_t size) {
    size_t new_size = res->size + size;

    if (new_size > res->cap) {

        size_t new_capacity = res->cap * 2;
        while (new_capacity < new_size) new_capacity *= 2;

        char* new_items = realloc(res->items, new_capacity);
        ASSERT_MEM(new_items);

        res->cap = new_capacity;
        res->items = new_items;
    }

    memcpy(res->items + res->size, buf, size);
    res->size = new_size;
}

void response_append_cstr(Response* res, const char* str) {
    size_t size = strlen(str);
    response_append_buf(res, str, size);
}

void response_free(Response* res) {
    if (!res->items) return;
    
    free(res->items);
    res->items = NULL;
}


void create_server_socket(Server* server) {
    server->addr_len = sizeof(struct sockaddr_in);
    server->fd = socket(AF_INET, SOCK_STREAM, 0);
    ASSERT_SUCCESS(server->fd, "Socket creation failed");
}

void set_server_ip(Server* server, char* ip) {
    server->addr.sin_family = AF_INET;
    server->addr.sin_addr.s_addr = INADDR_ANY;
    //server->addr.sin_addr.s_addr = inet_addr(ip);
    server->ip = strdup(ip);
}

void set_server_port(Server* server, int port) {
    server->addr.sin_port = htons(port);
    server->port = port;
}

void bind_and_listen(Server* server) {
    int bind_flag = bind(server->fd, (struct sockaddr*)&server->addr, server->addr_len);
    ASSERT_CLOSE_SOCK(bind_flag, server->fd, "Bind failed");

    printf("Connected to server at %s:%d\n", server->ip, server->port);

    int listen_flag = listen(server->fd, INCOMING_CONNECTIONS);
    ASSERT_CLOSE_SOCK(listen_flag, server->fd, "Listen failed");

    printf("Server listening on port %d...\n\n", server->port);
}

void close_server(Server* server) {
    close(server->fd);
    free(server->ip);
    printf("Closed server\n");
}

void* client(void* arg) {

    int client_fd = *((int*) arg);
    char in_buffer[BUFFER_SIZE];

    int read_size = recv(client_fd, in_buffer, sizeof(in_buffer), 0);
    if (read_size > 0) {
        in_buffer[read_size] = '\0';
        printf("Received: %s\n", in_buffer);
    }

    char lens[16];
    const char* m = "\
    <!DOCTYPE html>\n\
    <html lang=\"en\">\n\
        <head>\n\
            <meta charset=\"UTF-8\">\n\
            <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n\
            <title>Server</title>\n\
        </head>\n\
    <body>\n\
        <h1>Welcome!</h1>\n\
        <p>We'll be right back.</p>\n\
    </body>\n\
    </html>\n";
    sprintf(lens, "%d", (int)strlen(m));

    Response res = {0};
    response_alloc(&res);
    response_append_cstr(&res, "HTTP/1.1 200 OK\n");
    response_append_cstr(&res, "Content-Type: text/html; charset=UTF-8\n");
    response_append_cstr(&res, "Content-Length: ");
    response_append_cstr(&res, lens);
    response_append_cstr(&res, "\n\n");
    response_append_cstr(&res, m);

    int send_flag = send(client_fd, res.items, res.size, 0);
    ASSERT_CLOSE_SOCK(send_flag, client_fd, "Send failed");

    close(client_fd);
    response_free(&res);
    free(arg);

    return NULL;
}


int main() {

    Server server = {0};
    create_server_socket(&server);
    set_server_ip(&server, SERVER_IP);
    set_server_port(&server, SERVER_PORT);
    bind_and_listen(&server);

    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);

        int* client_fd_p = malloc(sizeof(int));

        *client_fd_p = accept(server.fd, (struct sockaddr*)&client_addr, &client_len);
        if (*client_fd_p < 0) {
            perror("Accept failed");
            continue;
        }

        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
        printf("Client connected from %s:%d\n", client_ip, ntohs(client_addr.sin_port));

        pthread_t pthread_id;
        pthread_create(&pthread_id, NULL, &client, (void*)client_fd_p);
        pthread_detach(pthread_id);
    }

    close_server(&server);


    return 0;
}
