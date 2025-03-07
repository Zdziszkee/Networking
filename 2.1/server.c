#include <stdio.h>
#include <stdlib.h>
#define _POSIX_C_SOURCE 200809L
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  if (argc != 2) {
    fprintf(stderr, "Usage: %s <port>\n", argv[0]);
    return EXIT_FAILURE;
  }
  int port = atoi(argv[1]);
  // UDP DATAGRAM SOCK_DGRAM
  int server_socket_descriptor = socket(AF_INET, SOCK_DGRAM, 0);

  if (server_socket_descriptor < 0) {
    fprintf(stderr, ("Socket creation failed"));
    return EXIT_FAILURE;
  }

  struct sockaddr_in socket_address = {
      .sin_family = AF_INET,
      .sin_addr = {.s_addr = htonl(INADDR_ANY)}, // any address
      .sin_port = htons(port)                    // use port from argument
  };

  int bind_result =
      bind(server_socket_descriptor, (struct sockaddr *)&socket_address,
           sizeof(socket_address));
  if (bind_result == -1) {
    fprintf(stderr, "Could not bind to socket! \n");
    return EXIT_FAILURE;
  }

  // UDP doesn't need listen() as it's connectionless

  while (true) {
    unsigned char buffer[1024];
    struct sockaddr_in client_address;
    socklen_t client_address_len = sizeof(client_address);

    // Receive data from client
    ssize_t number_of_bytes_read =
        recvfrom(server_socket_descriptor, buffer, sizeof(buffer), 0,
                 (struct sockaddr *)&client_address, &client_address_len);

    if (number_of_bytes_read == -1) {
      fprintf(stderr, "Could not receive data! \n");
      continue; // Try again with the next packet
    }

    fprintf(stdout, "Received %zd bytes\n", number_of_bytes_read);

    // Prepare and send response
    const char *response = "Hello, world!\r\n";
    size_t response_len = strlen(response);
    memset(buffer, 0, sizeof(buffer)); // clear buffer
    memcpy(buffer, response, response_len);

    ssize_t write_result =
        sendto(server_socket_descriptor, buffer, response_len, 0,
               (struct sockaddr *)&client_address, client_address_len);

    if (write_result == -1) {
      fprintf(stderr, "Could not send to socket! \n");
      continue; // Try again with the next packet
    }

    fprintf(stdout, "Sent %zd bytes\n", write_result);
  }

  int server_close_result = close(server_socket_descriptor);
  if (server_close_result == -1) {
    fprintf(stderr, "Could not close socket! \n");
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
