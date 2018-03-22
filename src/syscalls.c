#include <errno.h>
#include <stdio.h>
#include <sys/unistd.h>

#include "usart.h"

int _write(int file, char *ptr, int len);

int _write(int file, char *ptr, int len) {
  int i;

  if (file == STDOUT_FILENO || file == STDERR_FILENO) {
    for (i = 0; i < len; i++) {
      usart_write(ptr[i]);
    }

    return i;
  }

  errno = EIO;
  return -1;
}
