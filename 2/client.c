// TCP/IPv4 client skeleton.
//
// Replacing SOCK_STREAM with SOCK_DGRAM turns this into a UDP/IPv4 client
// using a socket in connection mode.

#define _POSIX_C_SOURCE 200809L
#include <arpa/inet.h>
#include <ctype.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
int main(int argc, char *argv[]) {

  // Check command line arguments
  if (argc != 3) {
    fprintf(stderr, "Usage: %s server_address port\n", argv[0]);
    return 1;
  }

  // Parse server address and port from command line
  const char *server_address = argv[1];
  int server_port = atoi(argv[2]);

  // Validate port number
  if (server_port <= 0 || server_port > 65535) {
    fprintf(stderr, "Invalid port number. Must be between 1 and 65535.\n");
    return 1;
  }

  // Create socket
  int client_socket_descriptor = socket(AF_INET, SOCK_STREAM, 0);
  if (client_socket_descriptor == -1) {
    perror("socket");
    return 1;
  }

  // Set up server address structure
  struct sockaddr_in addr = {.sin_family = AF_INET,
                             .sin_port = htons(server_port)};

  // Convert IP address from text to binary form
  if (inet_pton(AF_INET, server_address, &addr.sin_addr) <= 0) {
    fprintf(stderr, "Invalid address: %s\n", server_address);
    close(client_socket_descriptor);
    return 1;
  }

  // Connect to server
  printf("Connecting to %s:%d...\n", server_address, server_port);
  int connect_result =
      connect(client_socket_descriptor, (struct sockaddr *)&addr, sizeof(addr));
  if (connect_result == -1) {
    fprintf(stderr, "Failed to connect");
    close(client_socket_descriptor);
    return 1;
  }
  printf("Connected successfully\n");

  // Prepare and send data
  unsigned char buf[16];
  memcpy(buf, "ping", 4);

  ssize_t bytes_written = write(client_socket_descriptor, buf, 4);
  if (bytes_written == -1) {
    fprintf(stderr, "Failed to send");
    close(client_socket_descriptor);
    return 1;
  }
  printf("Sent %zi bytes\n", bytes_written);

  // Receive response
  ssize_t bytes_read = read(client_socket_descriptor, buf, 16);
  if (bytes_read == -1) {
    fprintf(stderr, "Failed to read");
    close(client_socket_descriptor);
    return 1;
  }
  printf("Received %zi bytes: ", bytes_read);

  // Print received data, verifying each byte is printable or a control
  // character
  for (ssize_t i = 0; i < bytes_read; i++) {
    unsigned char c = buf[i];

    if (isprint(c) || c == '\n' || c == '\r' || c == '\t') {
      printf("%c", c);
    } else {

      printf("[Not printable character %d]", c);
    }
  }
  printf("\n");

  // Close socket
  int close_result = close(client_socket_descriptor);
  if (close_result == -1) {
    fprintf(stderr, "Failed to close socket");
    return 1;
  }

  return 0;
}
