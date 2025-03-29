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

bool is_valid_input(const unsigned char *str, size_t length) {
  for (size_t i = 0; i < length; i++) {
    // Check for null bytes, tabs, carriage returns, and line feeds
    if (str[i] == 0 || str[i] == '\t' || str[i] == '\r' || str[i] == '\n') {
      return false;
    }

    // Check if character is not an alphabetic character or space
    if (!isalpha(str[i]) && str[i] != ' ') {
      // Check if the byte is part of a UTF-8 multibyte sequence
      if ((str[i] & 0xC0) != 0x80 && (str[i] & 0xC0) != 0xC0) {
        return false;
      }
    }
  }
  return true;
}

bool has_valid_format(const char *str) {
  int len = strlen(str);

  // Check if string begins with space
  if (len > 0 && isspace((unsigned char)str[0])) {
    return false;
  }

  // Check if string ends with space
  if (len > 0 && isspace((unsigned char)str[len - 1])) {
    return false;
  }

  // Check for multiple spaces
  for (int i = 0; i < len - 1; i++) {
    if (isspace((unsigned char)str[i]) && isspace((unsigned char)str[i + 1])) {
      return false;
    }
  }

  return true;
}

int main(int argc, char *argv[]) {
  // Hardcoded port to 2020
  int port = 2020;
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
        recvfrom(server_socket_descriptor, buffer, sizeof(buffer), 0,
                 (struct sockaddr *)&client_address, &client_address_len);

    if (number_of_bytes_read == -1) {
      fprintf(stderr, "Could not receive data! \n");
      continue;
    }

    // Create a null-terminated copy for string operations
    char safe_buffer[65507];
    memcpy(safe_buffer, buffer, number_of_bytes_read);
    safe_buffer[number_of_bytes_read] = '\0';

    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(client_address.sin_addr), client_ip, INET_ADDRSTRLEN);
    printf("Received from client %s:%d: \"%s\"\n", client_ip,
           ntohs(client_address.sin_port), safe_buffer);

    const char *error_message = "ERROR";

    printf("Processing message: \"%s\"\n", safe_buffer);

    // Validate input - check both the raw bytes and the string format
    if (!is_valid_input(buffer, number_of_bytes_read) ||
        !has_valid_format(safe_buffer)) {
      sendto(server_socket_descriptor, error_message, strlen(error_message), 0,
             (struct sockaddr *)&client_address, client_address_len);
      continue;
    }

    int word_count = 0;
    int palindrome_count = 0;

    char *token = strtok(safe_buffer, " ");
    while (token != NULL) {
      if (strlen(token) > 0) {
        word_count++;

        if (is_palindrome(token)) {
          palindrome_count++;
          printf("Palindrome found: \"%s\"\n", token);
        }
      }

      token = strtok(NULL, " ");
    }

    char response[256];
    int resp_len = snprintf(response, sizeof(response), "%d/%d",
                            palindrome_count, word_count);
    if (resp_len < 0 || resp_len >= (int)sizeof(response)) {
      fprintf(stderr, "Response message formatting failed or was truncated!\n");
      continue;
    }

    ssize_t write_result =
        sendto(server_socket_descriptor, response, resp_len, 0,
               (struct sockaddr *)&client_address, client_address_len);

    if (write_result == -1) {
      fprintf(stderr, "Could not send to socket! \n");
    }
  }

  int server_close_result = close(server_socket_descriptor);
  if (server_close_result == -1) {
    fprintf(stderr, "Could not close socket! \n");
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
