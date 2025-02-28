#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
/** 5
W dokumentacji POSIX API znajdź opisy czterech podstawowych funkcji plikowego
wejścia-wyjścia, tzn. open, read, write i close. Czy zgadzają się one z tym, co
pamiętasz z przedmiotu „Systemy operacyjne”? Jakie znaczenie ma wartość 0
zwrócona jako wynik funkcji read?

1. `open`: This function is used to open a file descriptor. It takes a filename
and flags as arguments, and returns a file descriptor if successful, or -1 if an
error occurs.

2. `read`: This function reads data from a file descriptor into a buffer. It
takes a file descriptor, a buffer, and the number of bytes to read as arguments.
It returns the number of bytes read, or -1 if an error occurs. A return value of
0 indicates that the end of the file has been reached.

3. `write`: This function writes data from a buffer to a file descriptor. It
takes a file descriptor, a buffer, and the number of bytes to write as
arguments. It returns the number of bytes written, or -1 if an error occurs.

4. `close`: This function closes a file descriptor. It takes a file descriptor
as an argument and returns 0 if successful, or -1 if an error occurs.

*/

/** 6
Zaimplementuj program kopiujący dane z pliku do pliku przy pomocy powyższych
funkcji. Zakładamy, że nazwy plików są podawane przez użytkownika jako argumenty
programu (tzn. będą dostępne w tablicy argv). Zwróć szczególną uwagę na obsługę
błędów — każde wywołanie funkcji we-wy musi być opatrzone testem sprawdzającym,
czy zakończyło się ono sukcesem, czy porażką. Funkcje POSIX zwracają -1 aby
zasygnalizować wystąpienie błędu, i dodatkowo zapisują w globalnej zmiennej
errno kod wskazujący przyczynę wystąpienia błędu (na dysku nie ma pliku o takiej
nazwie, brak wystarczających praw dostępu, itd.). Polecam Państwa uwadze
pomocniczą funkcję perror, która potrafi przetłumaczyć ten kod na zrozumiały dla
człowieka komunikat i wypisać go na ekranie.
*/

const mode_t default_file_mode = 0644;   // Owner read/write, others read
const size_t default_buffer_size = 1024; // Buffer size for reading/writing

const size_t open_input_file(const char *file_path) {
  size_t file_descriptor = open(file_path, O_RDONLY);
  if (file_descriptor == -1) {
    fprintf(stderr, "Failed to open input '%s': %s\n", file_path,
            strerror(errno));
    return EXIT_FAILURE;
  }
  return file_descriptor;
}

const size_t open_output_file(const char *file_path) {
  size_t file_descriptor =
      open(file_path, O_WRONLY | O_CREAT | O_TRUNC, default_file_mode);
  if (file_descriptor == -1) {
    fprintf(stderr, "Failed to open output '%s': %s\n", file_path,
            strerror(errno));
    return EXIT_FAILURE;
  }
  return file_descriptor;
}

const size_t read_file_content(int file_descriptor, char *buffer,
                               size_t max_bytes) {
  ssize_t bytes_read = read(file_descriptor, buffer, max_bytes);
  if (bytes_read == -1) {
    fprintf(stderr, "Failed to read file: %s\n", strerror(errno));
    return EXIT_FAILURE;
  }
  return bytes_read;
}

ssize_t write_file_content(int file_descriptor, const char *buffer,
                           size_t byte_count) {
  ssize_t bytes_written = write(file_descriptor, buffer, byte_count);
  if (bytes_written == -1) {
    fprintf(stderr, "Failed to write file: %s\n", strerror(errno));
    return EXIT_FAILURE;
  }
  if (bytes_written < (ssize_t)byte_count) {
    fprintf(stderr, "Partial write: %zd of %zu bytes written\n", bytes_written,
            byte_count);
  }
  return bytes_written;
}
int main(int args_size, char **args) {
  if (args_size != 3) {
    perror("Wrong number of arguments, expected: <source_file> "
           "<destination_file>");
    return EXIT_FAILURE;
  }

  const char *source = args[1];
  const char *destination = args[2];

  int input_fd = open_input_file(source);
  if (input_fd == -1) {
    return EXIT_FAILURE;
  }

  // Open output file
  int output_fd = open_output_file(destination);
  if (output_fd == -1) {
    close(input_fd);
    return EXIT_FAILURE;
  }

  char buffer[default_buffer_size];
  ssize_t bytes_read;
  while ((bytes_read =
              read_file_content(input_fd, buffer, default_buffer_size)) > 0) {
    ssize_t bytes_written = write_file_content(output_fd, buffer, bytes_read);
    if (bytes_written == -1 || bytes_written < bytes_read) {
      fprintf(stderr, "Write failed or incomplete\n");
      break;
    }
    printf("Copied %zd bytes\n", bytes_written);
  }

  if (bytes_read == -1) {
    fprintf(stderr, "Read error occurred\n");
  }

  // Close both files
  if (close(input_fd) == -1) {
    fprintf(stderr, "Failed to close input file: %s\n", strerror(errno));
  }
  if (close(output_fd) == -1) {
    fprintf(stderr, "Failed to close output file: %s\n", strerror(errno));
  }

  return (bytes_read == -1) ? EXIT_FAILURE : EXIT_SUCCESS;
}
