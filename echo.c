#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define DEFAULT_PORT 46645
#define LISTEN_BACKLOG 5

void* handle_connection(void* a_client_ptr) {
  int a_client = *(int*)a_client_ptr;
  free(a_client_ptr);

  char buffer[1024];
  int bytes_read;

  while (1) {
    bytes_read = read(a_client, buffer, sizeof(buffer) - 1);
    if (bytes_read == 0) {
      printf("End of connection on socket_fd %d\n", a_client);
      break;
    }
    if (bytes_read < 0) {
      perror("Error reading from socket");
      break;
    }

    buffer[bytes_read] = '\0';
    printf("Received from socket_fd %d: %s\n", a_client, buffer);
    write(a_client, buffer, bytes_read);
  }

  close(a_client);
  return NULL;
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

  if (bind(socket_fd, (struct sockaddr*)&socket_address,
           sizeof(socket_address)) < 0) {
    perror("Bind failed");
    close(socket_fd);
    return 1;
  }

  if (listen(socket_fd, LISTEN_BACKLOG) < 0) {
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

    int* client_fd_ptr = malloc(sizeof(int));
    *client_fd_ptr = client_fd;

    pthread_t thread;
    if (pthread_create(&thread, NULL, handle_connection, client_fd_ptr) != 0) {
      perror("Could not create thread");
      free(client_fd_ptr);
      close(client_fd);
      continue;
    }

    pthread_detach(thread);
  }

  close(socket_fd);
  return 0;
}