#ifndef VOID_VECTOR_H
#define VOID_VECTOR_H

#include <stdlib.h>

struct void_vector {
  void* data;
  size_t capacity;
  size_t count;
  size_t t_size;
};

#endif
