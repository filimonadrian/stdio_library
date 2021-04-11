
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <stdint.h>
#include <string.h>
#include "../util/so_stdio.h"

#define BUFSIZE 4096
#define WRITE 2
#define READ 1
#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))

/* https://stackoverflow.com/questions/3437404/min-and-max-in-c */

typedef struct _so_file {
	char buffer[4096];
	size_t buflen;
	int fd;
	int mode;
	int update;
	int nth_ch;
	int last_op;
	long file_offset;
	long read_offset;
} SO_FILE;
