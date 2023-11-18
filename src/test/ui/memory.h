#ifndef MEMORY_H
#define MEMORY_H

#define ZeroStruct(val) ZeroMemory_((u8 *)&val, sizeof(val))
#define ZeroArray(array, count, T) ZeroMemory_((u8 *)array, count*sizeof(T))

void ZeroMemory_(u8 *src, memory_index size) {
    for (memory_index i = 0; i < size; ++i) {
        *src++ = 0;
    }
}

#define CopyMemory(dest, src, size) CopyMemory_((u8 *)dest, (u8 *)src, size)

function void CopyMemory_(u8 *dest, u8 *src, size_t size) {
    for (size_t i = 0; i < size; ++i) {
        *dest++ = *src++;
    }
}

typedef struct MemoryArena {
    memory_index size;
    u8 *base;
    memory_index used;
} MemoryArena;

function void InitializeArena(MemoryArena *arena, memory_index size, u8 *base) {
    arena->size = size;
    arena->base = base;
    arena->used = 0;
}

function void ZeroArena(MemoryArena *arena) {
    ZeroArray(arena->base, arena->used, u8);
    arena->used = 0;
}

#define PushStruct(arena, T) (T *)PushSize_(arena, sizeof(T))
#define PushArray(arena, count, T) (T *)PushSize_(arena, (count)*(sizeof(T)))

void *PushSize_(MemoryArena *arena, memory_index size) {
    Assert((arena->used + size) <= arena->size);
    void *result = arena->base + arena->used;
    arena->used += size;
    return result;
}

#endif
