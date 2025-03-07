// UDP/IPv4 client skeleton.
//
// Using SOCK_DGRAM for UDP communication without maintaining a connection.

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

  // Create socket (changed to SOCK_DGRAM for UDP)
  int client_socket_descriptor = socket(AF_INET, SOCK_DGRAM, 0);
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

  // UDP doesn't establish a connection, so we'll just log the destination
  printf("Sending to %s:%d...\n", server_address, server_port);

  // Prepare and send data
  unsigned char buf[16];
  memcpy(buf, "ping", 4);

  // Use sendto instead of write for UDP
  ssize_t bytes_written = sendto(client_socket_descriptor, buf, 4, 0,
                                 (struct sockaddr *)&addr, sizeof(addr));
  if (bytes_written == -1) {
    fprintf(stderr, "Failed to send");
    close(client_socket_descriptor);
    return 1;
  }
  printf("Sent %zi bytes\n", bytes_written);

  // Receive response
  struct sockaddr_in sender_addr;
  socklen_t sender_addr_len = sizeof(sender_addr);

  // Use recvfrom instead of read for UDP
  ssize_t bytes_read =
      recvfrom(client_socket_descriptor, buf, 16, 0,
               (struct sockaddr *)&sender_addr, &sender_addr_len);
  if (bytes_read == -1) {
    fprintf(stderr, "Failed to read");
    close(client_socket_descriptor);
    return 1;
  }

  // Get the sender's IP address as a string
  char sender_ip[INET_ADDRSTRLEN];
  inet_ntop(AF_INET, &(sender_addr.sin_addr), sender_ip, INET_ADDRSTRLEN);

  printf("Received %zi bytes from %s:%d: ", bytes_read, sender_ip,
         ntohs(sender_addr.sin_port));

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
