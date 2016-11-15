
#define mi_SKIP_POISON
#include "mi_includes.h"
#include"openjpeg.h"
static INLINE void *mi_aligned_alloc_n(size_t alignment, size_t size)
{
  void* ptr;

  /* alignment shall be power of 2 */
  assert( (alignment != 0U) && ((alignment & (alignment - 1U)) == 0U));
  /* alignment shall be at least sizeof(void*) */
  assert( alignment >= sizeof(void*));

  if (size == 0U) { /* prevent implementation defined behavior of realloc */
    return NULL;
  }

  /*
  * Generic aligned malloc implementation.
  * Uses size_t offset for the integer manipulation of the pointer,
  * as uintptr_t is not available in C89 to do
  * bitwise operations on the pointer itself.
  */
  alignment--;
  {
	  size_t offset;
	  mi_UINT8 *mem;

	  /* Room for padding and extra pointer stored in front of allocated area */
	  size_t overhead = alignment + sizeof(void *);

	  /* let's be extra careful */
	  assert(alignment <= (SIZE_MAX - sizeof(void *)));

	  /* Avoid integer overflow */
	  if (size > (SIZE_MAX - overhead)) {
		  return NULL;
	  }

	  mem = (mi_UINT8*)malloc(size + overhead);
	  if (mem == NULL) {
		  return mem;
	  }
	  /* offset = ((alignment + 1U) - ((size_t)(mem + sizeof(void*)) & alignment)) & alignment; */
	  /* Use the fact that alignment + 1U is a power of 2 */
	  offset = ((alignment ^ ((size_t)(mem + sizeof(void*)) & alignment)) + 1U) & alignment;
	  ptr = (void *)(mem + sizeof(void*) + offset);
	  ((void**)ptr)[-1] = mem;
  }

  return ptr;
}
static INLINE void *mi_aligned_realloc_n(void *ptr, size_t alignment, size_t new_size)
{
  void *r_ptr;

  /* alignment shall be power of 2 */
  assert( (alignment != 0U) && ((alignment & (alignment - 1U)) == 0U));
  /* alignment shall be at least sizeof(void*) */
  assert( alignment >= sizeof(void*));

  if (new_size == 0U) { /* prevent implementation defined behavior of realloc */
    return NULL;
  }

  if (ptr == NULL) {
	  return mi_aligned_alloc_n(alignment, new_size);
  }
  alignment--;
  {
	  void *oldmem;
	  mi_UINT8 *newmem;
	  size_t overhead = alignment + sizeof(void *);

	  /* let's be extra careful */
	  assert(alignment <= (SIZE_MAX - sizeof(void *)));

	  /* Avoid integer overflow */
	  if (new_size > SIZE_MAX - overhead) {
		  return NULL;
	  }

	  oldmem = ((void**)ptr)[-1];
	  newmem = (mi_UINT8*)realloc(oldmem, new_size + overhead);
	  if (newmem == NULL) {
		  return newmem;
	  }

	  if (newmem == oldmem) {
		  r_ptr = ptr;
	  }
	  else {
		  size_t old_offset;
		  size_t new_offset;

		  /* realloc created a new copy, realign the copied memory block */
		  old_offset = (size_t)((mi_UINT8*)ptr - (mi_UINT8*)oldmem);

		  /* offset = ((alignment + 1U) - ((size_t)(mem + sizeof(void*)) & alignment)) & alignment; */
		  /* Use the fact that alignment + 1U is a power of 2 */
		  new_offset = ((alignment ^ ((size_t)(newmem + sizeof(void*)) & alignment)) + 1U) & alignment;
		  new_offset += sizeof(void*);
		  r_ptr = (void *)(newmem + new_offset);

		  if (new_offset != old_offset) {
			  memmove(newmem + new_offset, newmem + old_offset, new_size);
		  }
		  ((void**)r_ptr)[-1] = newmem;
	  }
  }

	return r_ptr;
}
void * mi_malloc(size_t size)
{
  if (size == 0U) { /* prevent implementation defined behavior of realloc */
    return NULL;
  }
  return malloc(size);
}
void * mi_calloc(size_t num, size_t size)
{
  if (num == 0 || size == 0) {
    /* prevent implementation defined behavior of realloc */
    return NULL;
  }
  return calloc(num, size);
}

void *mi_aligned_malloc(size_t size)
{
  return mi_aligned_alloc_n(16U, size);
}
void * mi_aligned_realloc(void *ptr, size_t size)
{
  return mi_aligned_realloc_n(ptr, 16U, size);
}

void mi_aligned_free(void* ptr)
{
}

void * mi_realloc(void *ptr, size_t new_size)
{
  if (new_size == 0U) { /* prevent implementation defined behavior of realloc */
    return NULL;
  }
  return realloc(ptr, new_size);
}
void mi_free(void *ptr)
{
  free(ptr);
}
