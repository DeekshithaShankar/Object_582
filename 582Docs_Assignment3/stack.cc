#include "BLI_stack.h"
#include <cassert>  // For assert
#include <cstdlib>  // For malloc, free
#include <cstring>  // For memcpy

// StackChunk structure for storing stack data.
struct StackChunk {
  StackChunk *next;
  char data[1];  // Flexible array member for chunk data.
};

// BLI_Stack structure.
struct BLI_Stack {
  size_t elem_size;
  size_t chunk_size;
  size_t chunk_index;
  size_t element_count;
  StackChunk *top_chunk;
};

// Create a new stack with specified element size and chunk size.
BLI_Stack *BLI_stack_new_ex(size_t elem_size, const char *description, size_t chunk_size)
{
  assert(elem_size > 0);
  BLI_Stack *stack = (BLI_Stack *)malloc(sizeof(BLI_Stack));
  stack->elem_size = elem_size;
  stack->chunk_size = chunk_size > 0 ? chunk_size : 1024 * elem_size;
  stack->chunk_index = 0;
  stack->element_count = 0;
  stack->top_chunk = nullptr;
  return stack;
}

// Create a new stack with default chunk size.
BLI_Stack *BLI_stack_new(size_t elem_size, const char *description)
{
  return BLI_stack_new_ex(elem_size, description, 1024 * elem_size);
}

// Free the stack and its chunks.
void BLI_stack_free(BLI_Stack *stack)
{
  assert(stack);
  StackChunk *chunk = stack->top_chunk;
  while (chunk) {
    StackChunk *next = chunk->next;
    free(chunk);
    chunk = next;
  }
  free(stack);
}

// Push an element onto the stack (uninitialized memory).
void *BLI_stack_push_r(BLI_Stack *stack)
{
  assert(stack);
  if (!stack->top_chunk || stack->chunk_index == stack->chunk_size) {
    StackChunk *new_chunk = (StackChunk *)malloc(sizeof(StackChunk) + stack->chunk_size - 1);
    new_chunk->next = stack->top_chunk;
    stack->top_chunk = new_chunk;
    stack->chunk_index = 0;
  }
  void *ptr = &stack->top_chunk->data[stack->chunk_index];
  stack->chunk_index += stack->elem_size;
  stack->element_count++;
  return ptr;
}

// Push an element onto the stack (copying memory).
void BLI_stack_push(BLI_Stack *stack, const void *src)
{
  assert(stack && src);
  void *dst = BLI_stack_push_r(stack);
  memcpy(dst, src, stack->elem_size);
}

// Pop an element from the stack.
void BLI_stack_pop(BLI_Stack *stack, void *dst)
{
  assert(stack);
  if (stack->chunk_index == 0) {
    StackChunk *old_chunk = stack->top_chunk;
    assert(old_chunk);  // Ensure we have a chunk to pop from.
    stack->top_chunk = old_chunk->next;
    free(old_chunk);
    stack->chunk_index = stack->chunk_size;
  }
  stack->chunk_index -= stack->elem_size;
  stack->element_count--;
  if (dst) {
    memcpy(dst, &stack->top_chunk->data[stack->chunk_index], stack->elem_size);
  }
}

// Peek at the top element of the stack without popping it.
void *BLI_stack_peek(BLI_Stack *stack)
{
  assert(stack && stack->top_chunk);
  size_t offset = stack->chunk_index - stack->elem_size;
  return &stack->top_chunk->data[offset];
}

// Discard the top element from the stack.
void BLI_stack_discard(BLI_Stack *stack)
{
  assert(stack);
  if (stack->chunk_index == 0) {
    StackChunk *old_chunk = stack->top_chunk;
    assert(old_chunk);
    stack->top_chunk = old_chunk->next;
    free(old_chunk);
    stack->chunk_index = stack->chunk_size;
  }
  stack->chunk_index -= stack->elem_size;
  stack->element_count--;
}

// Clear the stack without freeing it.
void BLI_stack_clear(BLI_Stack *stack)
{
  assert(stack);
  while (stack->top_chunk) {
    StackChunk *old_chunk = stack->top_chunk;
    stack->top_chunk = old_chunk->next;
    free(old_chunk);
  }
  stack->chunk_index = 0;
  stack->element_count = 0;
}

// Pop multiple elements from the stack.
void BLI_stack_pop_n(BLI_Stack *stack, void *dst, unsigned int n)
{
  assert(stack && dst);
  for (unsigned int i = 0; i < n; i++) {
    BLI_stack_pop(stack, (char *)dst + i * stack->elem_size);
  }
}

// Pop multiple elements in reverse order.
void BLI_stack_pop_n_reverse(BLI_Stack *stack, void *dst, unsigned int n)
{
  assert(stack && dst);
  for (unsigned int i = n; i > 0; i--) {
    BLI_stack_pop(stack, (char *)dst + (i - 1) * stack->elem_size);
  }
}

// Get the count of elements in the stack.
size_t BLI_stack_count(const BLI_Stack *stack)
{
  assert(stack);
  return stack->element_count;
}

// Check if the stack is empty.
bool BLI_stack_is_empty(const BLI_Stack *stack)
{
  assert(stack);
  return stack->element_count == 0;
}
