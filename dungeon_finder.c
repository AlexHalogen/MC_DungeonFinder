#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

#include <arpa/inet.h> // for convenient endianess conversion
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/sysinfo.h>

// #include <zlib.h>
#include <pthread.h>
#include "queue.h"
#include "auto_array.h"
#include "utils/callbacks.h"
#include "cNBT/nbt.h"
#include "cNBT/list.h"


#define ZLIB_CHUNK_SIZE 40960
#define INSERT_RESULT(data_structure, data) queue_enqueue(data_structure, data)
#define EXTRACT_RESULT(data_structure) queue_dequeue(data_structure)


typedef enum {
	TYPE_UNKNOWN, TILE_ENTITY, BLOCK, ENTITY
} search_type;

struct params {
	int num_threads;
	char *target_path;
	/* Block, TileEntity, Entity */
	char *lookup_type;
	/* Terminated by NULL*/
	auto_array *include_ids;
	/* Terminated by NULL*/
	auto_array *exclude_ids;
};

struct thread_job {
	queue *q;
	int fd;
};

struct thread_arg {
	queue *q;
	queue *results;
	struct params *param;
};

struct spawner_info {
	int x;
	int y;
	int z;
	char *name;
};

void process_chunk(char *data, int length);
auto_array* CNBT_process_chunk(void *data, struct params *param, int length);
int handle_zlib_rc(int rc);
void *thread_worker(void *arg);


/* local util functions */
int parse_args(int argc, char **argv, struct params *p);
void print_usage();
void print_verbose_usage();
/* Python-like string splitting function. Callers should free memory associated with the vector*/
auto_array *string_split(const char *str, char delimiter);



static inline void print_stderr(const char* msg) {
	fprintf(stderr, "%s\n", msg);
}


void *thread_worker(void *arg) {
	queue *q = ((struct thread_arg*)arg)->q;
	// auto_array *r = ((struct thread_job*)arg)->r;
	queue *results = ( (struct thread_arg*)arg)->results;
	struct params *param = ( (struct thread_arg*)arg)->param;
	while (1) {

		struct thread_job *job = queue_dequeue(q);
		if (job == NULL) break;

		int fd = job->fd;
		off_t size = lseek(fd, 0, SEEK_END);
		lseek(fd, 0, SEEK_SET);

		char *buf = mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0);
		for (int i=0; i<1024; i++) { // max 1024 chunks in one region
			int *current = (int*)buf + i;

			int offset = ntohl( *current) >> 8; // convert big-endian to local endianess
			if (offset == 0) {
				// the chunk is not present, skip it
				continue;
			}

			if (4096*(size_t)offset >= size) {
				print_stderr("Malformed chunk file: offset too large");
				break;
			}

			int length = (*current) >> 24;
			// offset is give by 4 * ((x & 31) + (z & 31) * 32)
			unsigned short rawx = i & 0x1F;
			unsigned short rawz = i >> 5;
			signed char posx = ( (((unsigned char)0) - (unsigned char)((rawx & 0x10) >> 4)) & 0xE0 ) | rawx;
			signed char posz = ( (((unsigned char)0) - (unsigned char)((rawz & 0x10) >> 4)) & 0xE0 ) | rawz;

			// calculate address of chunk header
			int *head = (int*)(buf + (4096*offset));

			int chunk_length = *head; // first 4 bytes contain compressed chunk data -1 byte
			chunk_length = ntohl(chunk_length); // convert endianess
			(void)length; (void)posx; (void)posz;
			// printf("(%d, %d) @ %d, offset %d, size: %d\n", (int)posx, (int)posz, length, offset, chunk_length);
			auto_array *spawners = CNBT_process_chunk(((char*)head)+5, param, chunk_length-1);
			for (size_t i=0; i<array_size(spawners);i++) {
				struct spawner_info *si = *((struct spawner_info**)array_at(spawners, i));
				// printf("%s spawner found at (%d, %d, %d)\n", si->name, si->x, si->y, si->z);
				// free(si->name);
				// free(si);
				INSERT_RESULT(results, si);
			}
			array_destroy(spawners);
		}

		munmap(buf, size);
		close(fd);
		free(job);
	}
	return NULL;	
}


int main(int argc, char** argv) {
	
	char *dirname = NULL;
	struct params prms;
	memset(&prms, 0, sizeof(struct params) );
	int parse_rc = parse_args(argc, argv, &prms);
	if ( parse_rc == -1) {
		print_usage();
		exit(1);
	} else if (parse_rc == 1) {
		print_verbose_usage();
		exit(0);
	}
	dirname = prms.target_path;
	int num_threads = prms.num_threads;

	queue * q = queue_create(8);
	queue *results = queue_create(-1);

	struct thread_arg arg;
	arg.q = q;
	// arg.r = v;
	arg.results = results;
	arg.param = &prms;
	pthread_t* pids = malloc(sizeof(pthread_t) * num_threads);
	
	for (int i=0; i<num_threads; i++) {
		pthread_create(&pids[i], NULL, thread_worker, &arg);

	}

	DIR *dir = opendir(dirname);
	if (!dir) exit(1);
	struct dirent *f;
	int fd_dir = open(dirname, O_RDONLY);
	while ( ( f = readdir(dir) ) != NULL  ) {
		
		struct stat st;
		int rc = fstatat(fd_dir, f->d_name, &st, 0);
		if (rc < 0 || !S_ISREG(st.st_mode)) {
			continue;
		}
		int fd = openat(fd_dir, f->d_name, O_RDONLY);

		if (fd < 2) {
			fprintf(stderr,"fd\n");

		}
		struct thread_job *j = malloc(sizeof(struct thread_job));
		j->fd = fd;

		queue_enqueue(q, j);
	}


	for (int i=0; i<num_threads; i++) {
		queue_enqueue(q, NULL);
	}	

	for (int i=0; i<num_threads; i++) {
		pthread_join(pids[i], NULL);

	}

	queue_enqueue(results, NULL);
	struct spawner_info *info;
	while ( (info = EXTRACT_RESULT(results)) ) {

		printf("%s spawner found at (%d, %d, %d)\n", info->name, info->x, info->y, info->z);
		free(info->name);
		free(info);

	}
	close(fd_dir);
	closedir(dir);

	queue_destroy(q);
	queue_destroy(results);
	// array_destroy(v);

	free(pids);
	return 0;



}


auto_array* CNBT_process_chunk(void *data, struct params *param, int length) {

	nbt_node *root = nbt_parse_compressed(data, length);
	if (!root) {
		fprintf(stderr,"%d\n", errno);
		fprintf(stderr, "NBT parse error\n");
		return NULL;
	}
	
	auto_array *ret = array_create(pointer_copy_constructor2, pointer_destructor, pointer_default_constructor2, sizeof(void*));

	nbt_node *tileEntities = nbt_find_by_path(root, ".Level.TileEntities");
	if (tileEntities->type != TAG_LIST) {
		print_stderr("Level.TileEntities is not a list tag!");
	}

	struct list_head *list = &(tileEntities->payload.tag_list->entry);
	const struct list_head *current = NULL;
	int idx = 0; // for debugging
	
	for (current=list->flink; current!=list; current=current->flink) { // each list item is an entity entry
		nbt_node *cmpd = list_entry(current, struct nbt_list, entry)->data;
		if (cmpd->type != TAG_COMPOUND) {
			print_stderr("Level.TileEntites.x is not a compound tag!");
		}
		const struct list_head *current2 = NULL;
		int x = 0;
		int y = 0;
		int z = 0;
		char is_spawner = 0;
		char *mob_type = "TO BE IMPLEMENTED";
		struct spawner_info* info = NULL;

		// The Loop body is a total mess. 
		// TODO: Restructure to reduce boilerplates
		// TODO: find a way to support searching of Entities and Blocks too 
		list_for_each(current2, &cmpd->payload.tag_list->entry) { // recurse inside entity entry
			nbt_node *item = list_entry(current2, struct nbt_list, entry)->data;
			if (item->name && strcmp(item->name, "id") == 0) {
				if (item->type != TAG_STRING) {
					print_stderr("id tag not a string");
					break;
				}
				char *entity_id = item->payload.tag_string;
				if (strcmp(entity_id, "minecraft:mob_spawner") == 0) {
					is_spawner = 1;
				} else {
					continue;
				}
			}

			if (item->name && strcmp(item->name, "SpawnData") == 0) {

				nbt_node *id = nbt_find_by_path(item, "SpawnData.id");
				if (!id) {
					print_stderr("SpawnData Malformed");
					break;
				}

				if (id->type != TAG_STRING) {
					print_stderr("spawner id tag not a string");
					break;
				}
				mob_type = id->payload.tag_string;

				auto_array *include_ids = param->include_ids;
				if (include_ids) {
					int not_wanted = 1; // if empty include then select all
					// since include/exclude lists are small, searching linearly is faster than hashtables
					for (size_t i=0; i<array_size(include_ids); i++) {
						if (strcmp(mob_type, *((char**)array_at(include_ids, i)) ) == 0) {
							not_wanted = 0;
							break;
						}
					}
					if (not_wanted) break; // break out of current tile entity	
				}
				
				auto_array *exclude_ids = param->exclude_ids;
				if (exclude_ids) {
					int not_wanted = 0;
					for (size_t i=0; i<array_size(exclude_ids); i++) {
						if (strcmp(mob_type, *((char**)array_at(exclude_ids, i))) == 0) {
							not_wanted = 1;
							break;
						}
					}
					if (not_wanted) break; // break out of current tile entity	
				}
				



				
			}

			if (item->name && strcmp(item->name, "x") == 0) {
				if (item->type != TAG_INT) {
					print_stderr("x tag not a int");
				}
				x = item->payload.tag_int;
			}
			
			if (item->name && strcmp(item->name, "y") == 0) {
				if (item->type != TAG_INT) {
					print_stderr("y tag not a int");
				}
				y = item->payload.tag_int;
			}

			if (item->name && strcmp(item->name, "z") == 0) {
				if (item->type != TAG_INT) {
					print_stderr("z tag not a int");
				}
				z = item->payload.tag_int;
			}


		}

		if (is_spawner) {
			info = malloc(sizeof(struct spawner_info));
			info->x = x;
			info->y = y;
			info->z = z;
			info->name = strdup(mob_type);
			array_push_back(ret, info);
			// printf("%s spawner found at (%d, %d, %d)\n", mob_type, x, y, z);
		}

		idx++; // for debugging
	}

	nbt_free(root);
	return ret;
}

/* This is the remain of previous attempts
int handle_zlib_rc(int rc) {

	switch(rc) {

		case Z_OK:
			return 0;
		
		case Z_MEM_ERROR:
			fprintf(stderr, "Z_MEM_ERROR\n");
			return 1;
			
		case Z_BUF_ERROR:
			fprintf(stderr, "Z_BUF_ERROR\n");
			return 1;

		case Z_DATA_ERROR:
			fprintf(stderr, "Z_DATA_ERROR\n");
			return 1;

	}
	return 1;

}
*/

int parse_args(int argc, char **argv, struct params *p) {
	if (argc < 2) return -1;
	int num_threads = get_nprocs();
	int idx = 1;
	while (idx < argc) {
		char *arg = argv[idx];
		if (strcmp(arg, "-t") == 0) { // reading next arg
			idx++;
			if (idx >= argc-1) return -1;
			char *str_num = argv[idx];
			char *endptr;
			long nt = strtol(str_num, &endptr, 10);
			if (*endptr != 0 ) {
				return -1;
			}
			num_threads = nt;
		} 
		else if (strcmp(arg, "-i") == 0) {
			if (p->include_ids) return -1; // duplicate -i options
			idx++;
			if (idx >= argc-1) return -1;
			char *include_list = argv[idx];
			auto_array *ret = string_split(include_list, ',');
			p->include_ids = ret;
		}
		else if (strcmp(arg, "-e") == 0) {
			if (p->exclude_ids) return -1; // duplicate -e options
			idx++;
			if (idx >= argc-1) return -1;
			char *exclude_list = argv[idx];
			auto_array *ret = string_split(exclude_list, ',');
			p->exclude_ids = ret;
		}
		else if (strcmp(arg, "-h") == 0) {
			return 1;
		}
		else {
			if (idx != argc-1) return -1;
			p->target_path = arg;
		}
		idx++;
	}

	// p->target_path = argv[argc-1]; // region dir should be the last param
	p->num_threads = num_threads;

	return 0;
}


void print_usage() {
	print_stderr("usage: ./dungeon_finder [-t num_threads] [-i includeid1,id2,... | -e exclude_id1,id2,...] <region directory|.mca file>");
	print_stderr("run './dungeon_finder -h' for detailed help");
}

void print_verbose_usage() {
		printf("usage: ./dungeon_finder [-t num_threads] [-i includeid1,id2,... | -e exclude_id1,id2,...] <region directory|.mca file>\n");
		printf("\n\t-t : number of threads to run with; defaults to number of logical CPUs\n");
		printf("\n\t-i : ids to be included in result list. Should not be used together(logically) with -e.\n");
		printf("\n\t-e : ids to be excluded in result list. Should not be used together(logically) with -i.\n");
		printf("\n\tRegion file directory should be the last input\n");	

}

// Python-like string splitting function. Callers should free memory associated with the vector
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
