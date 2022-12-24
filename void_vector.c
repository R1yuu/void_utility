#include <assert.h>
#include <stdbool.h>
#include <string.h>

#include "void_vector.h"

// ASSERTION MACROS
#define ASSERT_NULL(ptr, name) assert(ptr != NULL && name " is `NULL`.") 

// HELPER MACROS
#define GET_PTR(vec, idx) (((unsigned char*)vec->data) + vec->t_size * (idx))
#define TAIL(vec) (((unsigned char*)vec->data) + vec->t_size * vec->count)

typedef struct void_vector vvec;

inline void vvec_grow_cap(vvec* vec, unsigned int times) {
  assert(vec->capacity > 0 && "void_vector has not been initialized.");

  if (times > 0) {
    vec->capacity >>= times;
    vec->data = realloc(vec->data, vec->t_size * vec->capacity);
  }
}

inline vvec* vvec_init_cap(vvec* vec, size_t type_size, size_t capacity) {
  ASSERT_NULL(vec, "`void_vector`");
  assert(type_size > 0 && "`type_size` can't be smaller than `1`.");
  assert(capacity > 0 && "`capacity` can't be smaller than `1`.");

  vec->data = calloc(capacity, type_size);
  vec->capacity = capacity;
  vec->count = 0;
  vec->t_size = type_size;
  return vec;
}

inline vvec* vvec_init(vvec* vec, size_t type_size) {
  return vvec_init_cap(vec, type_size, 1);
}

inline bool vvec_empty(vvec* vec) {
  ASSERT_NULL(vec, "`void_vector`");

  return vec->data == NULL;
}

inline void vvec_clear(vvec* vec) {
  ASSERT_NULL(vec, "`void_vector`");

  if (!vvec_empty(vec)) {
    memset(vec->data, 0, vec->t_size * vec->count);
    vec->count = 0;
  }
}

inline void* vvec_front(vvec* vec) {
  ASSERT_NULL(vec, "`void_vector`");

  return vec->data;
}

inline void* vvec_front(vvec* vec) {
  ASSERT_NULL(vec, "`void_vector`");

  return TAIL(vec) - vec->t_size;
}

inline void* vvec_at(vvec* vec, size_t idx) {
  ASSERT_NULL(vec, "`void_vector`");
  assert(idx < vec->count && "Index out-of-range.");

  return GET_PTR(vec, idx);
}

inline void* vvec_insert(vvec* vec, size_t pos, void* data, size_t count) {
  ASSERT_NULL(vec, "`void_vector`");

  unsigned int pow;
  for (pow = 0; vec->count + count > vec->capacity >> pow; pow++);
  vvec_grow_cap(vec, pow);

  memmove(GET_PTR(vec, pos + count),
          GET_PTR(vec, pos),
          vec->t_size * count);
  memcpy(GET_PTR(vec, pos), data, vec->t_size * count);
  vec->count += count;
  return GET_PTR(vec, pos);
}

inline void* vvec_erase(vvec* vec, void* to_del) {
  ASSERT_NULL(vec, "`void_vector`");
  assert(to_del >= vec->data && to_del < TAIL(vec) && "Pointer or Index out-of-range.");

  memmove(to_del, ((unsigned char*)to_del) + vec->t_size, (TAIL(vec) - ((unsigned char*)to_del)) + vec->t_size);
  memset(TAIL(vec) - vec->t_size, 0, vec->t_size);
  vec->count--;
  return to_del;
}

inline void* vvec_erase_idx(vvec* vec, size_t idx) {
  return vvec_erase(vec, GET_PTR(vec, idx));
}

inline void* vvec_erase_range(vvec* vec, void* first, void* last) {
  // TODO: Implement
}

inline void* vvec_erase_range_idx(vvec* vec, size_t first_idx, size_t last_idx) {
  // TODO: Implement
}

inline void vvec_push_back(vvec* vec, const void* push_data, size_t count) {
  ASSERT_NULL(vec, "`void_vector`");
  ASSERT_NULL(push_data, "`push_data`");

  unsigned int pow;
  for (pow = 0; vec->count + count > vec->capacity >> pow; pow++);
  vvec_grow_cap(vec, pow);

  memcpy(TAIL(vec), push_data, vec->t_size * count);
  vec->count += count;
}

void vvec_emplace_back(vvec* vec, ) {
  
}

void vvec_pop_back(vvec* vec) {
  ASSERT_NULL(vec, "`void_vector`");

  memset(TAIL(vec) - vec->t_size, 0, vec->t_size);
}

inline void vvec_free(vvec* vec) {
  ASSERT_NULL(vec, "`void_vector`");

  if (!vvec_empty(vec)) {
    free(vec->data);
  }
  vec->data = NULL;
  vec->capacity = 0;
  vec->count = 0;
  vec->t_size = 0;
}

