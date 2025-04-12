#include <ctype.h>
#include <netinet/in.h>
#include <regex.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define SERVER_PORT 2020
#define BUFFER_SIZE 2048
#define MAX_EVENTS 10
#define MAX_CLIENTS 1000
/*The backlog parameter defines the maximum length for the
     queue of pending connections.  If a connection request
     arrives with the queue full, the client may receive an error
     with an indication of ECONNREFUSED.  Alternatively, if the
     underlying protocol supports retransmission, the request may
     be ignored so that retries may succeed. */
#define BACKLOG 5

typedef struct {
  char buffer[BUFFER_SIZE];
  size_t pos;
} client_buffer_t;

// Function to check if a string is only letters and spaces
bool is_only_letters_and_spaces(const char *str, size_t length) {
  for (size_t i = 0; i < length; i++) {
    if (!(isalpha(str[i]) || isspace(str[i]))) {
      return false;
    }
  }
  return true;
}

bool has_valid_format(const char *str, int bytes_read) {
  regex_t regex;
  const char *pattern = "^[A-Za-z]+( [A-Za-z]+)*$";

  // Create a null-terminated temporary string
  char *temp = malloc(bytes_read + 1);
  if (!temp)
    return false;

  memcpy(temp, str, bytes_read);
  temp[bytes_read] = '\0';

  // Compile regex
  if (regcomp(&regex, pattern, REG_EXTENDED)) {
    free(temp);
    return false;
  }

  // Execute match
  int result = regexec(&regex, temp, 0, NULL, 0);

  // Cleanup
  regfree(&regex);
  free(temp);

  return result == 0;
}
// Function definition
bool is_palindrome(char *word) {
  size_t length = strlen(word);
  if (length <= 1) {
    return 1; // Single character or empty string is a palindrome
  }

  int left = 0;
  int right = length - 1;

  while (left < right) {
    char leftChar = tolower(word[left]);
    char rightChar = tolower(word[right]);

    if (leftChar != rightChar) {
      return false; // Not a palindrome
    }
    left++;
    right--;
  }
  return true; // Is a palindrome
}

void process_client_line(int client_fd, const char *line, size_t length) {
  // Remove \r\n from the end
  char *buffer = malloc(length + 1);
  if (!buffer)
    return;

  memcpy(buffer, line, length - 2); // Skip \r\n at the end
  buffer[length - 2] = '\0';

  if (length == 2) {
    write(client_fd, "0/0\r\n", 5);
    free(buffer);
    return;
  }

  bool is_valid = has_valid_format(buffer, length - 2);
  if (!is_only_letters_and_spaces(buffer, length - 2) || !is_valid) {
    write(client_fd, "ERROR\r\n", 7);
    free(buffer);
    return;
  }

  // Split string by spaces and process each word
  char *token = strtok(buffer, " ");
  int words_count = 0;
  int palindrome_words_count = 0;

  if (buffer[0] != '\0') {
    while (token != NULL) {
      words_count++;

      if (is_palindrome(token)) {
        palindrome_words_count++;
      }

      token = strtok(NULL, " "); // Get next word
    }
  }

  char response[20];
  snprintf(response, sizeof(response), "%d/%d\r\n", palindrome_words_count,
           words_count);
  write(client_fd, response, strlen(response));

  free(buffer);
}

int main(int argc, char *argv[]) {

  int domain = PF_INET;
  int type = SOCK_STREAM;
  int protocol = IPPROTO_TCP;
  int server_socket_descriptor = socket(domain, type, protocol);

  if (server_socket_descriptor == -1) {
    perror("Failed to create socket");
    exit(EXIT_FAILURE);
  }

  // Convert the port number to the required network byte order (uint16_t)
  // htons = Host TO Network Short
  uint16_t port_network_order = htons(SERVER_PORT);

  // Convert the "listen on any address" constant to network byte order
  // (uint32_t) htonl = Host TO Network Long INADDR_ANY represents 0.0.0.0 -
  // listen on all available interfaces
  uint32_t ip_address_network_order = htonl(INADDR_ANY);

  struct sockaddr_in server_addr = {
      // .sin_family: Specifies the address family.
      // AF_INET indicates that this structure holds an IPv4 address.
      .sin_family = AF_INET,
      // .sin_port: The port number for the socket.
      // MUST be in Network Byte Order (Big-Endian).
      .sin_port = port_network_order,
      // .sin_addr: A nested structure containing the IPv4 address.
      // Type: struct in_addr
      .sin_addr = {
          // .s_addr: The actual 32-bit IPv4 address within the nested struct.
          .s_addr = ip_address_network_order}};

  // Set socket option to reuse address
  int opt = 1;
  if (setsockopt(server_socket_descriptor, SOL_SOCKET, SO_REUSEADDR, &opt,
                 sizeof(opt)) < 0) {
    perror("setsockopt(SO_REUSEADDR) failed");
    return EXIT_FAILURE;
  }

  int bind_to_socket_result =
      bind(server_socket_descriptor, (struct sockaddr *)&server_addr,
           sizeof(server_addr));

  if (bind_to_socket_result == -1) {
    perror("Could not bind to socket!");
    return EXIT_FAILURE;
  }

  int listen_result = listen(server_socket_descriptor, BACKLOG);
  if (listen_result == -1) {
    perror("Failed to listen on server socket");
    exit(EXIT_FAILURE);
  }

  // creates event pool, takes in 0: No special behavior (what we used)
  // EPOLL_CLOEXEC: Set the close-on-exec flag for the new file descriptor
  // When this flag is set, the file descriptor will be automatically closed
  // if the process calls any of the exec family of functions (execve(),
  // execl(), execlp(), etc.)
  //  to execute a new program.
  int epoll_descriptor = epoll_create1(0);

  if (epoll_descriptor == -1) {
    perror("Failed to create epoll instance");
    return EXIT_FAILURE;
  }
  struct epoll_event server_descriptor_data_available_event = {
      // EPOLLIN means that event is called if data is available to read from
      // descriptor
      .events = EPOLLIN,
      .data = {.ptr = NULL, .fd = server_socket_descriptor}

  };
  // event pool control, we add listener for incoming data on server descriptor
  if (epoll_ctl(epoll_descriptor, EPOLL_CTL_ADD, server_socket_descriptor,
                &server_descriptor_data_available_event) == -1) {
    perror("Failed to add server socket descriptor to event pool");
    return EXIT_FAILURE;
  }

  // Create and initialize client buffers
  client_buffer_t *client_buffers =
      malloc(MAX_CLIENTS * sizeof(client_buffer_t));
  if (!client_buffers) {
    perror("Failed to allocate memory for client buffers");
    return EXIT_FAILURE;
  }
  for (int i = 0; i < MAX_CLIENTS; i++) {
    client_buffers[i].pos = 0;
  }

  // Event loop
  struct sockaddr_in client_addr;
  socklen_t client_addr_len = sizeof(client_addr);
  struct epoll_event events[MAX_EVENTS];
  char read_buffer[BUFFER_SIZE];
  while (1) {
    int events_emount = epoll_wait(epoll_descriptor, events, MAX_EVENTS,
                                   -1); // Wait indefinitely
    if (events_emount == -1) {
      perror("Failed to epoll wait events");
      return EXIT_FAILURE;
    }
    for (int i = 0; i < events_emount; i++) {
      int event_descriptor = events[i].data.fd;

      if (event_descriptor == server_socket_descriptor) {

        int client_descriptor =
            accept(server_socket_descriptor, (struct sockaddr *)&client_addr,
                   &client_addr_len);
        if (client_descriptor < 0) {
          perror("Accept failed");
          continue;
        }
        printf("New connection: fd %d\n", client_descriptor);

        // Initialize client buffer
        if (client_descriptor < MAX_CLIENTS) {
          client_buffers[client_descriptor].pos = 0;
        }

        struct epoll_event client_descriptor_data_available_event = {
            .events = EPOLLIN, .data = {.ptr = NULL, .fd = client_descriptor}

        };
        if (epoll_ctl(epoll_descriptor, EPOLL_CTL_ADD, client_descriptor,
                      &client_descriptor_data_available_event) < 0) {
          perror("Epoll add client_fd failed");
          close(client_descriptor);
          continue;
        }
      } else {
        // Data from a client
        int bytes_read = read(event_descriptor, read_buffer, BUFFER_SIZE - 1);

        if (bytes_read <= 0) {
          // Connection closed or error
          printf("Client fd %d disconnected\n", event_descriptor);
          epoll_ctl(epoll_descriptor, EPOLL_CTL_DEL, event_descriptor, NULL);
          close(event_descriptor);
        } else {
          // Process received data
          if (event_descriptor < MAX_CLIENTS) {
            client_buffer_t *buffer = &client_buffers[event_descriptor];

            // Add new data to buffer
            if (buffer->pos + bytes_read < BUFFER_SIZE) {
              memcpy(buffer->buffer + buffer->pos, read_buffer, bytes_read);
              buffer->pos += bytes_read;

              // Process complete lines
              size_t start = 0;
              for (size_t j = 0; j < buffer->pos - 1; j++) {
                if (buffer->buffer[j] == '\r' &&
                    buffer->buffer[j + 1] == '\n') {
                  // We found a complete line
                  size_t line_length = j - start + 2; // Include \r\n

                  process_client_line(event_descriptor, buffer->buffer + start,
                                      line_length);

                  start = j + 2; // Move past \r\n
                }
              }

              // If we processed some lines, remove them from the buffer
              if (start > 0) {
                memmove(buffer->buffer, buffer->buffer + start,
                        buffer->pos - start);
                buffer->pos -= start;
              }
            } else {
              // Buffer overflow, send error and clear buffer
              write(event_descriptor, "ERROR\r\n", 7);
              buffer->pos = 0;
            }
          }
        }
      }
    }
  }

  free(client_buffers);
  return EXIT_SUCCESS;
}
