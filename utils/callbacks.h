#pragma once

/* These are normal constructors/destructors that behave like those in C++ */
typedef void *(*copy_constructor_type)(void *elem);
typedef void (*destructor_type)(void *elem);
typedef void *(*default_constructor_type)(void);

/* These constructors create the object at the address pointed by dest instead of 
 * alloc-ing and returning addresses to new objects.
 *
 * Generally if these constructors are used(as in auto_array), nop_destructor() should be used. However, if 
 * certain postprocessing needs to be done, e.g destroying mutex locks, the user
 * call still pass in a destructor that does not free memory pointed by elem
 */
typedef void (*copy_constructor2_type)(void *dest, void *elem);
typedef void (*default_constructor2_type)(void *dest);

typedef size_t (*hash_function_type)(void *key);

typedef int (*compare_type)(const void *A, const void *B);


void nop_destructor(void *ptr);

void nop_default_constructor(void *dest);



/* These destructors and constructors never alloc or dealloc memory.
 * Used for shallow copies
 */
void pointer_destructor(void *ptr);

void *pointer_copy_constructor(void *ptr );
void *pointer_default_constructor();
void *pointer_copy_constructor(void *ptr );
void pointer_copy_constructor2(void *dest, void *ptr );
void pointer_default_constructor2(void *dest);
size_t pointer_hash_function(void *ptr);


/* Create, copy or dealloc a single char */
void char_destructor(void *ptr);
void *char_copy_constructor(void *ptr );
void *char_default_constructor();
void char_copy_constructor2(void *dest, void *ptr );
void char_default_constructor2(void *dest);
size_t char_hash_function(void *ptr);


/* Create, copy or dealloc types that are 4 bytes in size, e.g int, float */
void size4_destructor(void *ptr);
void *size4_copy_constructor(void *ptr );
void *size4_default_constructor();
void size4_copy_constructor2(void *dest, void *ptr );
void size4_default_constructor2(void *dest);
size_t size4_hash_function(void *ptr);

/* Create, copy or dealloc types that are 4 bytes in size, e.g size_t, double */
void size8_destructor(void *ptr);
void *size8_copy_constructor(void *ptr );
void *size8_default_constructor();
void size8_copy_constructor2(void *dest, void *ptr );
void size8_default_constructor2(void *dest);
size_t size8_hash_function(void *ptr);

/* Perform deep-copy of passed in string using strcpy() and return/copy pointers to
 * copied string
 * Will call free() to created string in destructors.
 */
void string_copy_constructor2(void *dest, void *elem);
void *string_copy_constructor(void *ptr );
void *string_default_constructor();
void string_destructor(void *elem);
void string_default_constructor2(void *dest);
size_t string_hash_function(void *elem) ;