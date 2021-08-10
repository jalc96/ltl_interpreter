/*
requirements
#include<stdio.h>
#include"utils.cpp"
*/
#define KILOBYTE 1024
#define MEGABYTE 1048576
#define GIGABYTE 1073741824

struct memory_arena {
    u64 size;
    u64 index;
    u8 *base;
};

#define str(s) #s

#define GET_MEMORY(type) (type*) get_memory(ARENA, sizeof(type));
#define GET_MEMORY_COUNT(type, count) (type*) get_memory(ARENA, count * sizeof(type));

void *get_memory(memory_arena *arena, u64 size) {
	char message[BUFFER_SIZE];
	sprintf(message, "Not enough memory, current size: %.2fMB (%.2fKB)", ((f32)arena->size) / (f32)MEGABYTE, ((f32)arena->size) / (f32)KILOBYTE);
	assert_msg((arena->index + size) <= arena->size, message);
	void *result = arena->base + arena->index;
	arena->index += size;

	return result;
}