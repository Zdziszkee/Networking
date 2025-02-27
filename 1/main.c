#include <stdio.h>
#include <string.h>

// Napisz program w C deklarujący w funkcji main tablicę int liczby[50]
// i wczytujący do niej z klawiatury kolejne liczby.
// Wczytywanie należy przerwać gdy użytkownik wpisze zero
// albo gdy skończy się miejsce w tablicy (tzn. po wczytaniu 50 liczb).
// Z main należy następnie wywoływać pomocniczą funkcję drukuj,
// przekazując jej jako argumenty adres tablicy oraz liczbę wczytanych do niej
// liczb. Funkcję tę zadeklaruj jako void drukuj(int tablica[], int
// liczba_elementow). W jej ciele ma być pętla for drukująca te elementy
// tablicy, które są większe od 10 i mniejsze od 100.

void print(int numbers[], int count) {
  for (int i = 0; i < count; i++) {
    if (numbers[i] > 10 && numbers[i] < 100) {
      printf("%d\n", numbers[i]);
    }
  }
}

int main() {
  int numbers[50];

  int i = 0;
  for (; i < 50; i++) {
    char buffer[32]; // Generous size for most integer inputs
    int num;
    if (fgets(buffer, sizeof(buffer), stdin) != NULL) {
      if (sscanf(buffer, "%d", &num) == 1) {
        numbers[i] = num;
        if (num == 0)
          break;
        printf("Read: %d\n", num);
      } else {
        printf("  Invalid input\n");
      }
    }
  }

  print(numbers, i);
  return 0;
}
