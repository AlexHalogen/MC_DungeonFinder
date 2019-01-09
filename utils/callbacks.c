#define _POSIX_C_SOURCE  200809L

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "callbacks.h"

void nop_destructor(void *ptr) {
	(void)ptr;
}

void nop_default_constructor(void *dest) {
	(void)dest;
}

void pointer_destructor(void *ptr) {
	(void)ptr;
}

void* pointer_copy_constructor(void *ptr ) {
	return ptr;
}

void pointer_copy_constructor2(void *dest, void *ptr ) {
	memcpy(dest, &ptr, sizeof(void*));
}


void pointer_default_constructor2(void *dest) {
	uint64_t zeros = 0;
	memcpy(dest, &zeros, sizeof(void*));
}

size_t pointer_hash_function(void *ptr) {
	return (size_t)ptr;
}


void char_destructor(void *ptr) {
	(void)ptr;
	 free(ptr);
}

void *char_copy_constructor(void *elem) {
	if (!elem) {
		return elem;
	}
	char *copy = malloc(sizeof(char));
	*copy = *((char *)elem);
	return copy;
}

void *char_default_constructor(void) {
	return calloc(1, sizeof(char));
}

void char_copy_constructor2(void *dest, void *ptr ) {
	memcpy(dest, ptr, 1);
}


void char_default_constructor2(void *dest) {
	*((char*)dest) = 0;
}

size_t char_hash_function(void *ptr) {
	return (size_t)(*((char*)ptr));
}




void size4_destructor(void *ptr) {
	(void)ptr;
	 free(ptr);
}

void *size4_copy_constructor(void *elem) {
	if (!elem) {
		return elem;
	}
	uint32_t *copy = malloc(sizeof(uint32_t));
	*copy = *((uint32_t*)elem);
	return copy;
}
void *size4_default_constructor(void) {
	return calloc(1, sizeof(uint32_t));
}

void size4_copy_constructor2(void *dest, void *ptr ) {
	memcpy(dest, ptr, 4);
}


void size4_default_constructor2(void *dest) {
	*((uint32_t*)dest) = 0;
}

size_t size4_hash_function(void *ptr) {
	return (size_t)(*((uint32_t*)ptr));
}


void size8_destructor(void *ptr) {
	(void)ptr;
	free(ptr);
}

void *size8_copy_constructor(void *elem) {
	if (!elem) {
		return elem;
	}
	uint64_t *copy = malloc(sizeof(uint64_t));
	*copy = *((uint64_t *)elem);
	return copy;
}
void *size8_default_constructor(void) {
	return calloc(1, sizeof(uint64_t));
}

void size8_copy_constructor2(void *dest, void *ptr ) {
	memcpy(dest, ptr, 8);
}


void size8_default_constructor2(void *dest) {
	*((uint64_t*)dest) = 0;
}

size_t size8_hash_function(void *ptr) {
	return (size_t)(*((uint64_t*)ptr));
}

void *string_copy_constructor(void *elem) {
	if (!elem)
		return NULL;
	return strdup(elem);
}



void *string_default_constructor(void) {
	return calloc(1, 1);
}

void string_copy_constructor2(void *dest, void *elem) {

	char *str = malloc(strlen((char*)elem) +1);
	strcpy(str, (char*)elem);

	memcpy(dest, str, sizeof(void*));
}

void string_destructor(void *elem) {
	free(elem);
}

void string_default_constructor2(void *dest) {
	char *str = calloc(1, 1);
	memcpy(dest, str, sizeof(void*));
}
/**
 * Modified from implementation presented here:
 * http://www.cse.yorku.ca/~oz/hash.html
 */
size_t string_hash_function(void *elem) {
	char *str = (char *)elem;
	if (!str) {
		return 163;
	}

	size_t hash = 5381;
	int c;
	while ((c = *str++)) {
		hash = (hash << 5) + hash + c;
	}

	return hash;
}