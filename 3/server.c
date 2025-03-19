#include <arpa/inet.h>
#include <ctype.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

bool is_palindrome(const char *str) {
  int len = strlen(str);
  int i, j;

  for (i = 0; i < len; i++) {
    if (isalnum(str[i]))
      break;
  }

  for (j = len - 1; j >= 0; j--) {
    if (isalnum(str[j]))
      break;
  }

  while (i < j) {
    if (tolower(str[i]) != tolower(str[j])) {
      return false;
    }
    do {
      i++;
    } while (i < len && !isalnum(str[i]));
    do {
      j--;
    } while (j >= 0 && !isalnum(str[j]));
  }

  return true;
}

bool is_valid_input(const char *str) {
  for (int i = 0; str[i] != '\0'; i++) {
    if (!isalpha(str[i]) && !isspace(str[i])) {
      return false;
    }
  }
  return true;
}

bool has_valid_format(const char *str) {
  int len = strlen(str);

  if (len > 0 && isspace(str[0])) {
    return false;
  }

  if (len > 0 && isspace(str[len - 1])) {
    return false;
  }

  return true;
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    fprintf(stderr, "Usage: %s <port>\n", argv[0]);
    return EXIT_FAILURE;
  }
  int port = atoi(argv[1]);
  int server_socket_descriptor = socket(AF_INET, SOCK_DGRAM, 0);

  if (server_socket_descriptor < 0) {
    fprintf(stderr, "Socket creation failed");
    return EXIT_FAILURE;
  }

  struct sockaddr_in socket_address = {
      .sin_family = AF_INET,
      .sin_addr = {.s_addr = htonl(INADDR_ANY)},
      .sin_port = htons(port)};

  int bind_result =
      bind(server_socket_descriptor, (struct sockaddr *)&socket_address,
           sizeof(socket_address));

  if (bind_result == -1) {
    fprintf(stderr, "Could not bind to socket! \n");
    return EXIT_FAILURE;
  }

  while (true) {

    unsigned char buffer[65507];
    struct sockaddr_in client_address;
    socklen_t client_address_len = sizeof(client_address);

    ssize_t number_of_bytes_read =
        recvfrom(server_socket_descriptor, buffer, sizeof(buffer) - 1, 0,
                 (struct sockaddr *)&client_address, &client_address_len);

    if (number_of_bytes_read == -1) {
      fprintf(stderr, "Could not receive data! \n");
      continue;
    }

    buffer[number_of_bytes_read] = '\0';

    // Print received message from client
    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(client_address.sin_addr), client_ip, INET_ADDRSTRLEN);
    printf("Received from client %s:%d: \"%s\"\n", client_ip,
           ntohs(client_address.sin_port), buffer);

    const char *error_msg = "ERROR\n";

    // Process each line separately
    char *saveptr;
    char *line = strtok_r((char *)buffer, "\n", &saveptr);

    while (line != NULL) {
      printf("Processing line: \"%s\"\n", line);

      if (!is_valid_input(line)) {
        sendto(server_socket_descriptor, error_msg, strlen(error_msg), 0,
               (struct sockaddr *)&client_address, client_address_len);
        line = strtok_r(NULL, "\n", &saveptr);
        continue;
      }

      if (!has_valid_format(line)) {
        sendto(server_socket_descriptor, error_msg, strlen(error_msg), 0,
               (struct sockaddr *)&client_address, client_address_len);
        line = strtok_r(NULL, "\n", &saveptr);
        continue;
      }

      char *input_copy = strdup(line);
      if (!input_copy) {
        fprintf(stderr, "Memory allocation failed\n");
        line = strtok_r(NULL, "\n", &saveptr);
        continue;
      }

      int word_count = 0;
      int palindrome_count = 0;

      char *word_saveptr;
      char *token = strtok_r(input_copy, " ", &word_saveptr);
      while (token != NULL) {
        if (strlen(token) > 0) {
          word_count++;

          if (is_palindrome(token)) {
            palindrome_count++;
            printf("Palindrome found: \"%s\"\n", token);
          }
        }

        token = strtok_r(NULL, " ", &word_saveptr);
      }

      free(input_copy);

      char response[256];
      int resp_len = snprintf(response, sizeof(response), "%d/%d\n",
                              palindrome_count, word_count);
      if (resp_len < 0 || resp_len >= (int)sizeof(response)) {
        fprintf(stderr,
                "Response message formatting failed or was truncated!\n");
        line = strtok_r(NULL, "\n", &saveptr);
        continue;
      }
      size_t response_len = (size_t)resp_len;

      ssize_t write_result =
          sendto(server_socket_descriptor, response, response_len, 0,
                 (struct sockaddr *)&client_address, client_address_len);

      if (write_result == -1) {
        fprintf(stderr, "Could not send to socket! \n");
      }

      // Process next line
      line = strtok_r(NULL, "\n", &saveptr);
    }
  }

  int server_close_result = close(server_socket_descriptor);
  if (server_close_result == -1) {
    fprintf(stderr, "Could not close socket! \n");
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
