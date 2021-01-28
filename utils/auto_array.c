#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "auto_array.h"





const int INITIAL_CAPACITY = 8;
const int GROWTH_FACTOR = 2;

struct auto_array {
	pthread_mutex_t mtx;
	copy_constructor2_type copy_constructor;
	destructor_type destructor;
	default_constructor2_type default_constructor;
	size_t size;
	size_t capacity;
	int elem_size;
	void *array_base;
};


#define LOCK_ARRAY(array) ( pthread_mutex_lock(&array->mtx) );
#define UNLOCK_ARRAY(array) ( pthread_mutex_unlock(&array->mtx) );
#define ELEM_AT(array, index)   ( ((char*)(array->array_base)) + index*(array->elem_size)  )


/**
 * Helper function that calculates the minimum size that can hold a amount of 
 * `target` elements.
 */
static size_t get_new_capacity(size_t target) {
	size_t new_capacity = 1;
	while (new_capacity < target) {
		new_capacity *= GROWTH_FACTOR;
	}
	return new_capacity;
}


// Caller should lock the array!
int array_internal_reserve(auto_array *array, size_t new_size) {

	int elem_size = array->elem_size;
	if (new_size > array->capacity) {
		size_t capacity = (array->capacity = get_new_capacity(new_size));
		void *new_array = realloc(array->array_base, 
							capacity * elem_size);

		if (new_array == NULL) {
			int ENOMEM = 12;
			return ENOMEM; // realloc failed, don't touch original array
		}
			
		array->array_base = new_array;
	}
	return 0;
}


auto_array *array_create(copy_constructor2_type copy_constructor,
						destructor_type destructor,
						default_constructor2_type default_constructor,
						int elem_size) {

	auto_array *array = malloc(sizeof(auto_array));
	pthread_mutex_init(&array->mtx, NULL);
	array->copy_constructor = copy_constructor;
	array->destructor = destructor;
	array->default_constructor = default_constructor;
	array->size = 0;
	array->capacity = INITIAL_CAPACITY;
	array->elem_size = elem_size;
	array->array_base = malloc(elem_size * INITIAL_CAPACITY);
	return array;
}


void array_destroy(auto_array *array) {
	#ifdef DEBUG
	assert(array);
	#endif
	pthread_mutex_destroy(&array->mtx);
	free(array->array_base);
	free(array);
}


int array_push_back(auto_array* array, void* elem) {
	#ifdef DEBUG
	assert(array);
	#endif
	LOCK_ARRAY(array);
	array_internal_reserve(array, array->size+1);

	array->copy_constructor(
			ELEM_AT(array, array->size),
			elem
	);
	array->size++;

	UNLOCK_ARRAY(array);
	return 0;
}

void array_pop_back(auto_array *array) {
	#ifdef DEBUG
	assert(array);
	#endif
	LOCK_ARRAY(array);
	if (array->size == 0) return;
	// array->destructor(array->array[array->size-1]);
	array->destructor( ELEM_AT(array, array->size-1) );
	array->size--;
	UNLOCK_ARRAY(array);
}




int array_reserve(auto_array *array, size_t new_size) {
	LOCK_ARRAY(array);

	int rc = array_internal_reserve(array, new_size);

	UNLOCK_ARRAY(array);

	return rc;
}



void array_insert(auto_array *array, size_t position, void *element) {
	#ifdef DEBUG
	assert(array);
	assert(element);
	#endif
	if (position >= array->size) {
		array_reserve(array, position+1);
		// initialize all positions past the last elem except insert position
		array_resize(array, position); 
		array->copy_constructor(ELEM_AT(array, position), element);
	} else {
		array_reserve(array, array->size+1);
		int elem_size = array->elem_size;
		memmove(ELEM_AT(array, position+1),
			ELEM_AT(array, position),
			(array->size - position) * elem_size
		);

		array->size++;
		array->copy_constructor(ELEM_AT(array, position), element);	
	}
}

void array_erase(auto_array *array, size_t position) {
	#ifdef DEBUG
	assert(array);
	assert(position < array_size(array));
	#endif

	int elem_size = array->elem_size;
	array->destructor(ELEM_AT(array, position));
	array->size--;
	memmove(ELEM_AT(array, position),
		ELEM_AT(array, position+1),
		(array->size - position) * elem_size
	);
}



void *array_front(auto_array *array) {
	#ifdef DEBUG
	assert(array);
	#endif
	return array->array_base;
}


void *array_back(auto_array *array) {
	#ifdef DEBUG
	assert(array);
	#endif
	return ELEM_AT(array, array->size-1);
}

/*void array_get(auto_array *array, size_t position, void *dest) {
	#ifdef DEBUG
	assert(array);
	assert(position < array->size);
	#endif
	memcpy(dest, ELEM_AT(array, position, array->elem_size), array->elem_size);
}*/

void *array_at(auto_array *array, size_t position) {
	#ifdef DEBUG
	assert(array);
	assert(position < array->size);
	#endif
	return ELEM_AT(array, position);
}


void array_set(auto_array *array, size_t position, void *element) {
	#ifdef DEBUG
	assert(array);
	assert(position < array->size);
	#endif

	
	array->destructor(ELEM_AT(array, position));
	array->copy_constructor(ELEM_AT(array,position), element);
}


void array_resize(auto_array *array, size_t n) {
	#ifdef DEBUG
	assert(array);
	#endif
	
	if (n < array->size) {
		for (size_t i=n; i<array->size; i++) {
			array->destructor(ELEM_AT(array, i));
		}
	} else {
		if (n > array->capacity) {
			array_reserve(array, n);
		}
		for (size_t i=array->size; i<n; i++) {
			array->default_constructor(ELEM_AT(array, i));
		}
	}
	array->size = n;
 
}


void array_clear(auto_array *array) {
    for (size_t i=0; i<array->size;i++) {
        array->destructor(ELEM_AT(array, i));
    }
    array->size = 0;
}

size_t array_size(auto_array *array) {
	return array->size;
}

size_t array_capacity(auto_array *array) {
	return array->capacity;
}

int array_empty(auto_array *array) {
	return array->size == 0;
}