#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define DEFAULT_PORT 46645
#define LISTEN_BACKLOG 5

void handle_connection(int a_client) {
  char buffer[1024];
  int bytes_read = read(a_client, buffer, sizeof(buffer));
  printf("Received %d bytes: %.*s\n", bytes_read, bytes_read, buffer);
  write(a_client, buffer, bytes_read);
}

int main(int argc, char* argv[]) {
  int port = DEFAULT_PORT;

  if (argc > 1) {
    if (strcmp(argv[1], "-p") == 0 && argc > 2) {
      port = atoi(argv[2]);
      if (port <= 0 || port > 65535) {
        fprintf(stderr, "Invalid port number: %s\n", argv[2]);
        return 1;
      }
    } else {
      fprintf(stderr, "Usage: %s -p port\n", argv[0]);
      return 1;
    }
  }

  int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (socket_fd < 0) {
    perror("Socket creation failed");
    return 1;
  }

  struct sockaddr_in socket_address;
  memset(&socket_address, 0, sizeof(socket_address));
  socket_address.sin_family = AF_INET;
  socket_address.sin_addr.s_addr = htonl(INADDR_ANY);
  socket_address.sin_port = htons(port);

  int return_value;

  return_value = bind(socket_fd, (struct sockaddr*)&socket_address,
                      sizeof(socket_address));
  if (return_value < 0) {
    perror("Bind failed");
    close(socket_fd);
    return 1;
  }

  return_value = listen(socket_fd, LISTEN_BACKLOG);
  if (return_value < 0) {
    perror("Listen failed");
    close(socket_fd);
    return 1;
  }

  printf("Server listening on port %d\n", port);

  struct sockaddr_in client_address;
  socklen_t client_address_len = sizeof(client_address);

  while (1) {
    int client_fd = accept(socket_fd, (struct sockaddr*)&client_address,
                           &client_address_len);
    if (client_fd < 0) {
      perror("Accept failed");
      continue;
    }

    handle_connection(client_fd);
    close(client_fd);
  }

  close(socket_fd);
  return 0;
}
