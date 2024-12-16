#include "connection.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_BUFFER_SIZE 1024

void handle_connection(int *sock_fd_ptr) {
    int sock_fd = *sock_fd_ptr;
    free(sock_fd_ptr);

    printf("Handling connection on %d\n", sock_fd);
    char buffer[MAX_BUFFER_SIZE];
    int bytes_read;

    while (1) {
        bytes_read = read(sock_fd, buffer, sizeof(buffer));
        if (bytes_read > 0) {
            buffer[bytes_read] = '\0';
            printf("Received From Connection %d: %s\n", sock_fd, buffer);

            if (strlen(buffer) == 0) {
                printf("Closing Connection on %d\n", sock_fd);
                break;
            }

            if (write(sock_fd, buffer, bytes_read) == -1) {
                perror("Write failed");
                break;
            }
        } else if (bytes_read == 0) {
            printf("Closing Connection on %d\n", sock_fd);
            break;
        } else {
            perror("Read failed");
            break;
        }
    }

    printf("Closing connection on %d\n", sock_fd);
    close(sock_fd);
}
