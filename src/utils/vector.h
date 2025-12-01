#ifndef __EDITOR_VECTOR_H
#define __EDITOR_VECTOR_H

#include <stddef.h>
#include <stdbool.h>

typedef void (*vector_destructor_fn)(void *elem);
typedef void (*vector_copy_fn)(void *dest, const void *src);

typedef struct Vector
{
    void *data;
    size_t elem_size;
    size_t size;
    size_t capacity;
    vector_destructor_fn dtor;   /* optional element destructor */
    vector_copy_fn copier;       /* optional element copier; defaults to memcpy */
} Vector;

void vector_init_s(Vector *v, size_t elem_size);
void vector_init_with_capacity_s(Vector *v, size_t elem_size, size_t capacity);
#define vector_init(v, T) vector_init_s((v), sizeof(T))
#define vector_init_with_capacity(v, T, c) vector_init_with_capacity_s((v), sizeof(T), (c))

void vector_set_destructor(Vector *v, vector_destructor_fn dtor);
void vector_set_copier(Vector *v, vector_copy_fn copier);

void vector_free(Vector *v);

size_t vector_size(const Vector *v);
size_t vector_capacity(const Vector *v);
bool vector_empty(const Vector *v);
int vector_reserve(Vector *v, size_t new_capacity);
int vector_shrink(Vector *v);

void *vector_data(Vector *v);
void *vector_at(Vector *v, size_t index);

int vector_push_back(Vector *v, const void *elem);
void *vector_emplace_back(Vector *v);
int vector_pop_back(Vector *v, void *out_elem);
int vector_insert(Vector *v, size_t index, const void *elem);
int vector_remove(Vector *v, size_t index, void *out_elem);
int vector_set(Vector *v, size_t index, const void *elem);
void vector_clear(Vector *v);

#endif /* __EDITOR_VECTOR_H */
