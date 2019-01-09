#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <pthread.h>
#include "utils/callbacks.h"
#include "queue.h"
#include "auto_array.h"

struct PTHREAD_ARGS {
	void *ptr1;
	void *ptr2;
	void *ptr3;
	pthread_mutex_t *mtx;
};


void* queue_worker(void * arg) {
	queue *q = ((struct PTHREAD_ARGS*)arg)->ptr3;
	int *counts = ((struct PTHREAD_ARGS*)arg)->ptr1;
	pthread_mutex_t* mtx = ((struct PTHREAD_ARGS*)arg)->mtx;
	while (1) {
		int *i = queue_dequeue(q);
		if (!i) break;
		pthread_mutex_lock(mtx);
		counts[*i] += 1;
		pthread_mutex_unlock(mtx);
		// printf("%d reported by tid %d\n", *i,  getpid());
	}
	
	return NULL;
}

int test_queue() {
	queue *q = queue_create(100);
	int *values = malloc(10000 * sizeof(int) );
	int *counts = calloc(10000, sizeof(int));
	pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
	struct PTHREAD_ARGS arg;
	arg.ptr1 = counts;
	arg.ptr2 = values;
	arg.ptr3 = q;
	arg.mtx = &mtx;

	pthread_t tids[8];
	

	for (int i=0; i<8; i++) {
		pthread_create( &tids[i], NULL, queue_worker, &arg );	
	}
	


	
	for (int i=0; i<10000; i++) {
		values[i] = i;
		queue_enqueue( q, &values[i]) ;
	}

	for (int i=0; i<8; i++) {
		queue_enqueue(q, NULL);
	}
	for (int i=0; i<8; i++) {
	
		pthread_join(tids[i], NULL);
	}
	int race = 0;
	for (int i=0; i<10000;i++) {
		if (counts[i] != 1) {
			fprintf(stderr, "Race condition occured\n");
			race = 1;
			break;
		}
	}

	if (!race) {
		printf("No data race in queue\n");
	}

	queue_destroy(q);
	free(values);
	free(counts);

	return 0;

}



auto_array *string_split(const char *str, char delimiter) {
    if (!str) return NULL;
    auto_array *ret = array_create(pointer_copy_constructor2, pointer_destructor, pointer_default_constructor2, sizeof(void*));

    const char *head = str;
    const char *tail = str;
    const char *end = str + strlen(str);
    while (tail < end) {
        if (*tail == delimiter) {
            char *slice = NULL;
            if ( head == tail ) { // continuous delimiters
                slice = calloc(1,sizeof(char));
            } else {
                slice = malloc((tail-head+1) * sizeof(char)); // tail includes delimiter;
                slice[tail-head] = 0;

                strncpy(slice, head, tail-head);
            }

            head = tail+1;
            tail = head;
            array_push_back(ret, slice);
        } else {
            tail++;
        }
    }
    // tail-head elems + \0
    // would be 0+1 if last elem is delimiter(head = tail)
    char *slice = calloc(tail-head+1, sizeof(char)); 
/*    for (size_t i=0; i< tail-head; i++) {
        slice[i] = array[head+i];
    }*/
    strncpy(slice, head, tail-head);
    array_push_back(ret, slice);
    return ret;
}


void test_split() {
char *strings[] = {
"a,b,c,d,e,f",
"a,b,c,d,e,f,",
",a,b,c,d,e,f",
",a,b,c,d,e,f,",
""
};

for (int i=0; i<5; i++) {
	auto_array *temp = string_split(strings[i], ',');
	for (size_t j=0; j<array_size(temp); j++) {
		printf("'%s', ", *((char**)array_at(temp, j) ));
		free(*((char**)array_at(temp, j)));
	}
	printf("\n");
	array_destroy(temp);

}




}

int main( int argc, char** argv) {
	test_queue();	
	test_split();


}
