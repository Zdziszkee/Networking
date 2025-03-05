#include <stdbool.h>
#include <stdio.h>
#include <string.h>
/** 1
Napisz program w C deklarujący w funkcji main tablicę int liczby[50]
i wczytujący do niej z klawiatury kolejne liczby.
Wczytywanie należy przerwać gdy użytkownik wpisze zero
albo gdy skończy się miejsce w tablicy (tzn. po wczytaniu 50 liczb).
Z main należy następnie wywoływać pomocniczą funkcję drukuj,
przekazując jej jako argumenty adres tablicy oraz liczbę wczytanych do niej
liczb. Funkcję tę zadeklaruj jako void drukuj(int tablica[], int
liczba_elementow). W jej ciele ma być pętla for drukująca te elementy
tablicy, które są większe od 10 i mniejsze od 100.
*/

void print(int numbers[], int count) {
  for (int i = 0; i < count; i++) {
    if (numbers[i] > 10 && numbers[i] < 100) {
      printf("%d\n", numbers[i]);
    }
  }
}

/** 2
Przypomnij sobie wiadomości o wskaźnikach i arytmetyce wskaźnikowej w C.
Napisz alternatywną wersję funkcji drukującej liczby,
o sygnaturze void drukuj_alt(int * tablica, int liczba_elementow).
Nie używaj w niej indeksowania zmienną całkowitoliczbową (nie może się więc
pojawić ani tablica[i], ani *(tablica+i)), zamiast tego użyj wskaźnika
przesuwanego z elementu na element przy pomocy ++. W dwóch następnych
zadaniach też używaj przesuwanego wskaźnika zamiast indeksowania zmienną
całkowitoliczbową.
*/
void print_alternative(const int *numbers, const int count) {

  for (int i = 0; i < count; i++) {
    int number = *numbers;
    if (number > 10 && number < 100) {
      printf("%d\n", number);
    }
    numbers++;
  }
}

/** 3
Opracuj funkcję sprawdzającą, czy przekazany jej bufor zawiera tylko i wyłącznie
drukowalne znaki ASCII, tzn. bajty o wartościach z przedziału domkniętego [32,
126]. Funkcja ma mieć następującą sygnaturę: bool is_printable_buf(const void *
buf, int len). Pamiętaj o włączeniu nagłówka <stdbool.h>, bez niego kompilator
nie rozpozna ani nazwy typu bool, ani nazw stałych true i false. Konieczne
będzie użycie rzutowania wskaźników, bo typ void * oznacza „adres w pamięci, ale
bez informacji o tym co w tym fragmencie pamięci się znajduje”. Na początku
ciała funkcji trzeba go więc zrzutować do typu „adres fragmentu pamięci
zawierającego ciąg bajtów”. Napisz też jakiś prosty program, który pozwoli Ci
przetestować działanie funkcji is_printable_buf.
*/
bool is_buffer_printable(const void *buffer, const int size) {
  const unsigned char *byteBuffer = (const unsigned char *)buffer;
  for (int i = 0; i < size; i++) {
    unsigned char character = *byteBuffer;
    if (character < 32 || character > 126) {
      return false;
    }
    byteBuffer++;
  }
  return true;
}

/** 4
Opracuj alternatywną wersję funkcji, biorącą jako argument łańcuch w sensie
języka C, czyli ciąg niezerowych bajtów zakończony bajtem równym zero (ten
końcowy bajt nie jest uznawany za należący do łańcucha). Ta wersja funkcji ma
mieć sygnaturę bool is_printable_str(const char * str).
*/
bool is_string_printable(const char *str) {
  const unsigned char *byteBuffer = (const unsigned char *)str;
  while (*byteBuffer != '\0') {
    unsigned char character = *byteBuffer;
    if (character < 32 || character > 126) {
      return false;
    }
    byteBuffer++;
  }
  return true;
}

int main() {
  int numbers[50];

  int i = 0;
  for (; i < 50; i++) {
    char buffer[32];
    int num;
    if (fgets(buffer, sizeof(buffer), stdin) != NULL) {
      if (sscanf(buffer, "%d", &num) == 1) {
        numbers[i] = num;
        printf("Read: %d\n", num);
        if (num == 0)
          break;
      } else {
        printf("  Invalid input\n");
      }
    }
  }

  print_alternative(numbers, i);
  return 0;
}
