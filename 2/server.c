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

  int server_socket_descriptor = socket(AF_INET, SOCK_STREAM, 0);

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

  int listen_result =
      listen(server_socket_descriptor,
             10); // 10 - backlog: The maximum number of pending connections
                  // that can be queued before new ones are refused.
  if (listen_result == -1) {
    fprintf(stderr, "Could not listen on socket! \n");
    return EXIT_FAILURE;
  }

  while (true) {
    int client_socket_descriptor = accept(server_socket_descriptor, NULL, NULL);
    if (client_socket_descriptor == -1) {
      fprintf(stderr, "Could not accept connection! \n");
      return EXIT_FAILURE;
    }

    unsigned char buffer[1024];

    // ssize_t number_of_bytes_read =
    //     read(client_socket_descriptor, buffer, sizeof(buffer));

    // fprintf(stdout, "Received %zd bytes: %s\n", number_of_bytes_read,
    // buffer);

    const char *response = "Hello, world!\r\n";
    size_t response_len = strlen(response);
    memset(buffer, 0, sizeof(buffer)); // clear buffer
    memcpy(buffer, response, response_len);

    ssize_t write_result =
        write(client_socket_descriptor, buffer,
              response_len); // nie wpisujemy całego bufora tylko długość
    // naszego słowa.. reszta to szum

    if (write_result == -1) {
      fprintf(stderr, "Could not write to socket! \n");
      return EXIT_FAILURE;
    }

    fprintf(stdout, "Sent %zd bytes\n", write_result);

    int client_close_result = close(client_socket_descriptor);
    if (client_close_result == -1) {
      fprintf(stderr, "Could not close socket! \n");
      return EXIT_FAILURE;
    }
  }
  int server_close_result = close(server_socket_descriptor);
  if (server_close_result == -1) {
    fprintf(stderr, "Could not close socket! \n");
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
