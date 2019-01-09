#pragma once
#include <stdlib.h>
#include "utils/callbacks.h"
/*
 * Simplistic array wrapping that allows automatic growing of the array
 * Array accepts any type of data, and data is guaranteed to be contigious in
 * memory, 1
 * e.g &(array_get(array, 0)) == &(array_get(array,1)) + sizeof(array_element)
 *
 * User of the array need to supply a destructor, a default constructor and
 * copy constructor.
 * Destructor behaves like those in C++, while copy constructor and default
 * constructor constructs the object at designated address(Assuming abundant
 * space is allocated)
 *
 * The array provides thread-safe push_back and pop_back operations, and
 * pop_back () does NOT block when the array is empty.
 *
 * All other operations are NOT thread-safe.
 */


typedef struct auto_array auto_array;


/*typedef void (*copy_constructor2_type)(void *dest, void *elem);

typedef void (*destructor_type)(void *elem);

typedef void (*default_constructor2_type)(void *dest);*/


// initial capacity for array
extern const int INITIAL_CAPACITY;
// factor used to expand the array when the array is full
extern const int GROWTH_FACTOR;


/*
 * Creates an auto_array "object"
 * default and constructor2 constructors create the object in *dest pointer
 * instead of returning a pointer to a malloc-ed new object
 * destructor behaves like normal destructors. Don't do anything in a
 * destructor for an array that stores plain values, e.g array of ints,
 * array of shallow copies
 *
 * @elem_size accepts the size of the object, e.g 4 for a int, 8 for a struct
 * of 2 ints
 */

auto_array *array_create(copy_constructor2_type copy_constructor,
					destructor_type destructor,
					default_constructor2_type default_constructor,
					int elem_size);

/*
 * Free memory of each element using specified destructor at array creation,
 * and then frees the array itself
 */
void array_destroy(auto_array *array);

/*
 * Pushes an object to the back of the array.
 * Copies the passed in object using copy-constructor specified at array
 * creation.
 * Thread safety: MT-safe
 */
int array_push_back(auto_array* array, void* elem);


/*
 * Delete the last object in the array using the destructor specified at array
 * creation. Shrinks the array size by 1.
 *
 * Do nothing and return immediately if the array is empty.
 *
 * Thread safety: MT-safe
 */
void array_pop_back(auto_array *array);

/*
 * Manually set the available space for storing elements
 * @new_size the new number of elements that the array can store
 *
 * Doesn't do anything and return immediately if the array has larger space
 * allocated than new_size * elem_size
 */
int array_reserve(auto_array *array, size_t new_size);


/*
 * Inserts an element at @position, moving original element at @position and
 * all elements after that towards the end of the array.
 * @element is copied by the copy constructor.
 *
 * Time complexity: O(n), where n is the number of elements after position.
 */
void array_insert(auto_array *array, size_t position, void *element);



/*
 * Removes an element at @position using the destructor, and move elements
 * after @position to the left.
 *
 * Time complexity: O(n), where n is the number of elements after position.
 */
void array_erase(auto_array *array, size_t position);


/*
 * Returns the pointer to the first element in the array.
 * Cast the returned pointer to desired types to get desired values.
 * The pointer can be indexed using square brackets to access elements in the
 * array, but don't free anything - no copies are done.
 *
 */
void *array_front(auto_array *array);

/*
 * Returns the pointer to the first element in the array.
 * Cast the returned pointer to desired types to get desired values.
 */
void *array_back(auto_array *array);


/*
 * Sets the element at @position to @element.
 * Existing element at that position will be removed using the desctuctor, and
 * @element is copied using copy constructor.
 *
 */
void array_set(auto_array *array, size_t position, void *element);

/*
 * Returns the pointer to the element at position.
 * Cast the returned pointer to desired types to get desired values.
 */
void *array_at(auto_array *array, size_t position);

/*
 * Resizes the array. Remove all elements beyond the new size using the
 * destructor if new_size is smaller, or create elements at positions beyond
 * the original size using the default constructor, then set the size of the
 * array to the new size.
 *
 * Will not change the allocated size of the internal array that stores
 * elements.
 *
 * Similar to stl::vector's resize().
 */
void array_resize(auto_array *array, size_t new_size);

/*
 * Delete all elements in the array using the destructor, and set the size of
 * the array to 0.
 *
 * Will not change the allocated size of the internal array that stores
 * elements.
 */
void array_clear(auto_array *array);

/*
 * Returns the size of the array.
 */
size_t array_size(auto_array *array);

/*
 * Returns the allocated size of the internal array that is used for storing
 * elements in terms of the max number of elements that could fit inside the
 * array.
 */
size_t array_capacity(auto_array *array);

/*
 * Returns 1 if the array is empty, 0 otherwise.
 */
int array_empty(auto_array *array);

/*
 * Macro used for getting the value of an element in the array.
 * @type: the type of the element, e.g int
 */
#define array_get(array, position, type) ( ((type*)(array_front(array)))[position] )
