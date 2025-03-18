#include <arpa/inet.h>
#include <ctype.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
int main(int argc, char *argv[]) {

  if (argc != 3) {
    fprintf(stderr, "Usage: %s server_address port\n", argv[0]);
    return 1;
  }

  const char *server_address = argv[1];
  int server_port = atoi(argv[2]);

  if (server_port <= 0 || server_port > 65535) {
    fprintf(stderr, "Invalid port number. Must be between 1 and 65535.\n");
    return 1;
  }

  int client_socket_descriptor = socket(AF_INET, SOCK_DGRAM, 0);
  if (client_socket_descriptor == -1) {
    perror("socket");
    return 1;
  }

  struct sockaddr_in addr = {.sin_family = AF_INET,
                             .sin_port = htons(server_port)};

  if (inet_pton(AF_INET, server_address, &addr.sin_addr) <= 0) {
    fprintf(stderr, "Invalid address: %s\n", server_address);
    close(client_socket_descriptor);
    return 1;
  }

  while (true) {
    char message[65507] = "";
    printf("Enter comma-separated words to be checked for palindromacy (press "
           "enter when done):\n");

    if (fgets(message, sizeof(message), stdin) == NULL) {
      continue;
    }

    size_t len = strlen(message);
    if (len > 0 && message[len - 1] == '\n') {
      message[len - 1] = '\0';
    }

    ssize_t msg_len = strlen(message);
    ssize_t bytes_written = sendto(client_socket_descriptor, message, msg_len,
                                   0, (struct sockaddr *)&addr, sizeof(addr));
    if (bytes_written == -1) {
      fprintf(stderr, "Failed to send");
      close(client_socket_descriptor);
      return 1;
    }

    struct sockaddr_in sender_addr;
    socklen_t sender_addr_len = sizeof(sender_addr);

    char response[256];
    ssize_t bytes_read =
        recvfrom(client_socket_descriptor, response, sizeof(response) - 1, 0,
                 (struct sockaddr *)&sender_addr, &sender_addr_len);

    if (bytes_read > 0) {
      response[bytes_read] = '\0';
    }
    if (bytes_read == -1) {
      fprintf(stderr, "Failed to read");
      close(client_socket_descriptor);
      return 1;
    }

    char sender_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(sender_addr.sin_addr), sender_ip, INET_ADDRSTRLEN);

    for (ssize_t i = 0; i < bytes_read; i++) {
      unsigned char c = response[i];
      printf("%c", c);
    }
    printf("\n");
  }

  int close_result = close(client_socket_descriptor);
  if (close_result == -1) {
    fprintf(stderr, "Failed to close socket");
    return 1;
  }
  return EXIT_SUCCESS;
}
