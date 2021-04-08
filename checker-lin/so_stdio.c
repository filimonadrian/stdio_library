#include "so_stdio.h"

SO_FILE *so_fopen(const char *pathname, const char *mode)
{
	SO_FILE *stream = malloc(sizeof(SO_FILE));
	int fd = 0;

	if (stream == NULL)
		exit(ENOMEM);

	memset(stream, 0, sizeof(SO_FILE));

	if (!strcmp(mode, "r+"))
		fd = open(pathname, O_RDWR);
	else if (!strcmp(mode, "w+"))
		fd = open(pathname, O_RDWR | O_CREAT | O_TRUNC);
	else if (!strcmp(mode, "a+"))
		fd = open(pathname, O_APPEND | O_RDONLY | O_CREAT);
	else if (!strcmp(mode, "r"))
		fd = open(pathname, O_RDONLY);
	else if (!strcmp(mode, "w"))
		fd = open(pathname, O_WRONLY | O_CREAT | O_TRUNC);
	else if (!strcmp(mode, "a"))
		fd = open(pathname, O_APPEND | O_CREAT);
	else {
		free(stream);
		return NULL;
	}

	if (fd <= 0){
		free(stream);
		return NULL;
	}

	stream->fd = fd;

	return stream;
}

int so_fclose(SO_FILE *stream)
{
	int ret = 0;
	int fd = 0;

	if (stream == NULL)
		return SO_EOF;

	fd = stream->fd;
	free(stream);

	if (fd < 0)
		return SO_EOF;

	ret = close(fd);
	if (ret != 0)
		return SO_EOF;

	return 0;
}


int so_fgetc(SO_FILE *stream)
{
	if (stream == NULL)
		return -1;

	ssize_t no_bytes = 0;
	unsigned int ch = 0;

	if (stream->nth_ch < stream->buflen) {
		ch = stream->buffer[stream->nth_ch];
		stream->nth_ch++;
		return ch;
	}

	memset(stream->buffer, 0, BUFSIZE);
	no_bytes = read(stream->fd, stream->buffer, BUFSIZE);
	stream->nth_ch = 0;

	if (no_bytes < 0)
		return SO_EOF;

	stream->buflen = no_bytes;

	ch = stream->buffer[stream->nth_ch];
	stream->nth_ch++;

	return ch;
}

int so_fputc(int c, SO_FILE *stream)
{
	return 0;
}

size_t so_fread(void *ptr, size_t size, size_t nmemb, SO_FILE *stream)
{
	return 0;
}

size_t so_fwrite(const void *ptr, size_t size, size_t nmemb, SO_FILE *stream)
{
	return 0;
}

int so_fseek(SO_FILE *stream, long offset, int whence)
{
	return 0;
}

long so_ftell(SO_FILE *stream)
{
	return 0;
}

int so_fflush(SO_FILE *stream)
{
	return 0;
}

int so_fileno(SO_FILE *stream)
{
	if (stream != NULL)
		return stream->fd;

	return -1;
}

int so_feof(SO_FILE *stream)
{
	return 0;
}

int so_ferror(SO_FILE *stream)
{
	return 0;
}

SO_FILE *so_popen(const char *command, const char *type)
{
	return NULL;
}

int so_pclose(SO_FILE *stream)
{
	return 0;
}

