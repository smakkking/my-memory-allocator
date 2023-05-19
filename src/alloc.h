#ifndef HEADER_1
#define HEADER_1

#include <cstddef>
using std::size_t;

void* my_malloc(size_t num);
void my_free(void *p);
void* my_calloc(size_t num, size_t nsize);
void* my_realloc(void *block, size_t size);

#endif