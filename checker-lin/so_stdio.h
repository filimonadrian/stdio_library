
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
#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
/* https://stackoverflow.com/questions/3437404/min-and-max-in-c */

typedef struct _so_file {
	char buffer[4096];
	size_t buflen;
	int fd;
	int nth_ch;

} SO_FILE;
