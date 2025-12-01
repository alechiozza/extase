#include "vector.h"

#include <stdlib.h>
#include <string.h>
#include <errno.h>

static void default_copy(void *dest, const void *src, size_t size)
{
    memcpy(dest, src, size);
}

static size_t next_capacity(size_t current, size_t needed)
{
    size_t new_cap = current ? current : 8;
    while (new_cap < needed)
    {
        size_t doubled = new_cap << 1;
        if (doubled <= new_cap)
            return needed;
        new_cap = doubled;
    }

    return new_cap;
}

void vector_init_s(Vector *v, size_t elem_size)
{
    v->data = NULL;
    v->elem_size = elem_size;
    v->size = 0;
    v->capacity = 0;
    v->dtor = NULL;
    v->copier = NULL;
}

void vector_init_with_capacity_s(Vector *v, size_t elem_size, size_t capacity)
{
    vector_init(v, elem_size);
    if (capacity)
        (void)vector_reserve(v, capacity);
}

void vector_set_destructor(Vector *v, vector_destructor_fn dtor)
{
    v->dtor = dtor;
}

void vector_set_copier(Vector *v, vector_copy_fn copier)
{
    v->copier = copier;
}

void vector_free(Vector *v)
{
    if (v->dtor)
    {
        for (size_t i = 0; i < v->size; ++i)
        {
            void *elem = (char *)v->data + i * v->elem_size;
            v->dtor(elem);
        }
    }
    free(v->data);
    v->data = NULL;
    v->size = 0;
    v->capacity = 0;
}

size_t vector_size(const Vector *v)
{
    return v->size;
}

size_t vector_capacity(const Vector *v)
{
    return v->capacity;
}

bool vector_empty(const Vector *v)
{
    return v->size == 0;
}

int vector_reserve(Vector *v, size_t new_capacity)
{
    if (new_capacity <= v->capacity)
        return 0;

    void *new_data = realloc(v->data, new_capacity * v->elem_size);
    if (!new_data)
        return -1;

    v->data = new_data;
    v->capacity = new_capacity;
    return 0;
}

int vector_shrink(Vector *v)
{
    if (v->size == v->capacity)
        return 0;
    
    if (v->size == 0)
    {
        free(v->data);
        v->data = NULL;
        v->capacity = 0;
        return 0;
    }

    void *new_data = realloc(v->data, v->size * v->elem_size);
    if (!new_data)
        return -1;
    v->data = new_data;
    v->capacity = v->size;
    return 0;
}

void *vector_data(Vector *v)
{
    return v->data;
}

static inline void do_copy(const Vector *v, void *dest, const void *src)
{
    if (v->copier)
        v->copier(dest, src);
    else
        default_copy(dest, src, v->elem_size);
}

void *vector_at(Vector *v, size_t index)
{
    if (index >= v->size)
        return NULL;
    return (char *)v->data + index * v->elem_size;
}

int vector_push_back(Vector *v, const void *elem)
{
    if (v->size == v->capacity)
    {
        size_t want = next_capacity(v->capacity, v->size + 1);
        if (vector_reserve(v, want) != 0)
            return -1;
    }

    void *dst = (char *)v->data + v->size * v->elem_size;
    do_copy(v, dst, elem);

    v->size++;

    return 0;
}

void *vector_emplace_back(Vector *v)
{
    if (v->size == v->capacity)
    {
        size_t want = next_capacity(v->capacity, v->size + 1);
        if (vector_reserve(v, want) != 0)
            return NULL;
    }

    void *dst = (char *)v->data + v->size * v->elem_size;
    
    v->size++;

    return dst;
}

int vector_pop_back(Vector *v, void *out_elem)
{
    if (v->size == 0)
        return -1;
    
    size_t idx = v->size - 1;
    void *elem = (char *)v->data + idx * v->elem_size;
    if (out_elem)
        default_copy(out_elem, elem, v->elem_size);
    
    if (v->dtor)
        v->dtor(elem);
    
    v->size--;

    return 0;
}

int vector_insert(Vector *v, size_t index, const void *elem)
{
    if (index > v->size)
        return -1;
    
    if (v->size == v->capacity)
    {
        size_t want = next_capacity(v->capacity, v->size + 1);
        if (vector_reserve(v, want) != 0)
            return -1;
    }

    void *pos = (char *)v->data + index * v->elem_size;
    void *next = (char *)pos + v->elem_size;
    size_t move_bytes = (v->size - index) * v->elem_size;
    memmove(next, pos, move_bytes);

    do_copy(v, pos, elem);

    v->size++;

    return 0;
}

int vector_remove(Vector *v, size_t index, void *out_elem)
{
    if (index >= v->size)
        return -1;
    
    void *pos = (char *)v->data + index * v->elem_size;
    if (out_elem)
        default_copy(out_elem, pos, v->elem_size);
    if (v->dtor)
        v->dtor(pos);
    
    void *next = (char *)pos + v->elem_size;
    size_t move_bytes = (v->size - index - 1) * v->elem_size;
    memmove(pos, next, move_bytes);

    v->size--;

    return 0;
}

int vector_set(Vector *v, size_t index, const void *elem)
{
    if (index >= v->size)
        return -1;
    
    void *dst = (char *)v->data + index * v->elem_size;
    if (v->dtor)
        v->dtor(dst);
    
    do_copy(v, dst, elem);

    return 0;
}

void vector_clear(Vector *v)
{
    if (v->dtor)
    {
        for (size_t i = 0; i < v->size; ++i)
        {
            void *elem = (char *)v->data + i * v->elem_size;
            v->dtor(elem);
        }
    }
    v->size = 0;
}
