#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

#define PORT 8080
#define BUFFER_SIZE 8192
#define MAX_CONTACTS 100

typedef struct {
    int id;
    char name[100];
    char phone[30];
} Contact;

Contact contact_book[MAX_CONTACTS];
int contact_count = 0;

void init_contacts() {
    contact_book[0].id = 1;
    strcpy(contact_book[0].name, "Alice Smith");
    strcpy(contact_book[0].phone, "123-456-7890");

    contact_book[1].id = 2;
    strcpy(contact_book[1].name, "Bob Jones");
    strcpy(contact_book[1].phone, "987-654-3210");

    contact_count = 2;
}

void get_contacts_json(char *response_body) {
    strcat(response_body, "[");

    for (int i = 0; i < contact_count; i++) {
        char item[256];

        sprintf(
            item,
            "{\"id\":%d,\"name\":\"%s\",\"phone\":\"%s\"}",
            contact_book[i].id,
            contact_book[i].name,
            contact_book[i].phone
        );

        strcat(response_body, item);

        if (i < contact_count - 1) {
            strcat(response_body, ",");
        }
    }

    strcat(response_body, "]");
}

void add_contact_from_json(const char *body) {
    if (contact_count >= MAX_CONTACTS) {
        return;
    }

    Contact c;
    c.id = contact_count + 1;

    char *name_ptr = strstr(body, "\"name\":\"");
    char *phone_ptr = strstr(body, "\"phone\":\"");

    if (!name_ptr || !phone_ptr) {
        return;
    }

    name_ptr += 8;
    phone_ptr += 9;

    char *name_end = strchr(name_ptr, '"');
    char *phone_end = strchr(phone_ptr, '"');

    if (!name_end || !phone_end) {
        return;
    }

    int name_len = (int)(name_end - name_ptr);
    int phone_len = (int)(phone_end - phone_ptr);

    if (name_len >= (int)sizeof(c.name))
        name_len = sizeof(c.name) - 1;

    if (phone_len >= (int)sizeof(c.phone))
        phone_len = sizeof(c.phone) - 1;

    strncpy(c.name, name_ptr, name_len);
    c.name[name_len] = '\0';

    strncpy(c.phone, phone_ptr, phone_len);
    c.phone[phone_len] = '\0';

    contact_book[contact_count++] = c;
}

int main() {
    WSADATA wsaData;
    SOCKET server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    char buffer[BUFFER_SIZE];

    init_contacts();

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("Winsock initialization failed.\n");
        return 1;
    }

    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd == INVALID_SOCKET) {
        printf("Socket creation failed.\n");
        WSACleanup();
        return 1;
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) == SOCKET_ERROR) {
        printf("Bind failed.\n");
        closesocket(server_fd);
        WSACleanup();
        return 1;
    }

    if (listen(server_fd, SOMAXCONN) == SOCKET_ERROR) {
        printf("Listen failed.\n");
        closesocket(server_fd);
        WSACleanup();
        return 1;
    }

    printf("C Backend Server running on http://localhost:%d\n", PORT);

    while (1) {
        new_socket = accept(
            server_fd,
            (struct sockaddr *)&address,
            &addrlen
        );

        if (new_socket == INVALID_SOCKET) {
            continue;
        }

        memset(buffer, 0, sizeof(buffer));

        int bytes_received = recv(
            new_socket,
            buffer,
            BUFFER_SIZE - 1,
            0
        );

        if (bytes_received <= 0) {
            closesocket(new_socket);
            continue;
        }

        buffer[bytes_received] = '\0';

        /* CORS Preflight */
        if (strncmp(buffer, "OPTIONS", 7) == 0) {
            const char *cors_response =
                "HTTP/1.1 204 No Content\r\n"
                "Access-Control-Allow-Origin: *\r\n"
                "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
                "Access-Control-Allow-Headers: Content-Type\r\n"
                "\r\n";

            send(
                new_socket,
                cors_response,
                (int)strlen(cors_response),
                0
            );
        }

        /* GET /contacts */
        else if (strncmp(buffer, "GET /contacts", 13) == 0) {
            char body[4096] = {0};
            char response[8192];

            get_contacts_json(body);

            sprintf(
                response,
                "HTTP/1.1 200 OK\r\n"
                "Content-Type: application/json\r\n"
                "Access-Control-Allow-Origin: *\r\n"
                "Content-Length: %d\r\n"
                "\r\n"
                "%s",
                (int)strlen(body),
                body
            );

            send(
                new_socket,
                response,
                (int)strlen(response),
                0
            );
        }

        /* POST /contacts */
        else if (strncmp(buffer, "POST /contacts", 14) == 0) {
            char *body = strstr(buffer, "\r\n\r\n");

            if (body) {
                body += 4;
                add_contact_from_json(body);
            }

            const char *response =
                "HTTP/1.1 201 Created\r\n"
                "Access-Control-Allow-Origin: *\r\n"
                "Content-Length: 0\r\n"
                "\r\n";

            send(
                new_socket,
                response,
                (int)strlen(response),
                0
            );
        }

        /* 404 Not Found */
        else {
            const char *response =
                "HTTP/1.1 404 Not Found\r\n"
                "Access-Control-Allow-Origin: *\r\n"
                "Content-Length: 0\r\n"
                "\r\n";

            send(
                new_socket,
                response,
                (int)strlen(response),
                0
            );
        }

        closesocket(new_socket);
    }

    closesocket(server_fd);
    WSACleanup();

    return 0;
}