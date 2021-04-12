#include "so_stdio.h"

/* returns number of readed bytes */
int read_buffer(SO_FILE *stream)
{
	int no_bytes = 0;

	if (stream == NULL)
		return -1;

	memset(stream->buffer, 0, BUFSIZE);
	stream->nth_ch = 0;

	no_bytes = read(stream->fd, stream->buffer, BUFSIZE);
	if (no_bytes < 0) {
		stream->err = 1;
		return SO_EOF;
	}

	if (no_bytes == 0) {
		stream->is_EOF = 1;
		return 0;
	}

	stream->buflen = no_bytes;

	return no_bytes;
}

void reset_buf(SO_FILE *stream)
{
	memset(stream->buffer, 0, BUFSIZE);
	stream->buflen = 0;
	stream->nth_ch = 0;
}

int write_buffer(SO_FILE *stream)
{
	if (stream == NULL)
		return SO_EOF;

	int no_bytes = 0;

	no_bytes = write(stream->fd, stream->buffer, stream->buflen);
	if (no_bytes < 0) {
		stream->err = 1;
		return SO_EOF;
	}

	reset_buf(stream);
	return no_bytes;
}


SO_FILE *so_fopen(const char *pathname, const char *mode)
{
	SO_FILE *stream = malloc(sizeof(SO_FILE));
	int fd = 0;

	if (stream == NULL)
		exit(ENOMEM);

	memset(stream, 0, sizeof(SO_FILE));

	if (!strcmp(mode, "r+")) {
		stream->mode = O_RDWR;
		stream->update = 1;
		fd = open(pathname, O_RDWR);
	} else if (!strcmp(mode, "w+")) {
		stream->mode = O_RDWR;
		stream->update = 1;
		fd = open(pathname, O_RDWR | O_CREAT | O_TRUNC);
	} else if (!strcmp(mode, "a+")) {
		stream->mode = O_APPEND;
		stream->update = 1;
		fd = open(pathname, O_APPEND | O_RDWR | O_CREAT);
	} else if (!strcmp(mode, "r"))
		fd = open(pathname, O_RDONLY);
	else if (!strcmp(mode, "w"))
		fd = open(pathname, O_WRONLY | O_CREAT | O_TRUNC);
	else if (!strcmp(mode, "a"))
		fd = open(pathname, O_WRONLY | O_APPEND | O_CREAT);
	else {
		free(stream);
		return NULL;
	}

	if (fd <= 0) {
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
	ssize_t no_bytes = 0;

	if (stream == NULL)
		return SO_EOF;

	// fprintf(stderr, "LAST_OP = %d\n", stream->last_op);
	if (stream->last_op == WRITE) {
		no_bytes = write_buffer(stream);
		if (no_bytes < 0) {
			stream->err = 1;
			free(stream);
			return SO_EOF;
		}
	}

	fd = stream->fd;
	free(stream);

	ret = close(fd);
	if (ret != 0)
		return SO_EOF;

	return 0;
}

int so_fgetc(SO_FILE *stream)
{
	if (stream == NULL)
		return SO_EOF;

	unsigned int ch = 0;
	ssize_t no_bytes = 0;

	stream->last_op = READ;

	if (stream->nth_ch >= stream->buflen) {
		no_bytes = read_buffer(stream);
		if (no_bytes <= 0) {
			stream->err = 1;
			return SO_EOF;
		}
	}

	ch = stream->buffer[stream->nth_ch];
	stream->nth_ch++;

	return ch;
}

int so_fputc(int c, SO_FILE *stream)
{
	if (stream == NULL)
		return SO_EOF;

	ssize_t no_bytes = 0;

	stream->last_op = WRITE;

	if (stream->buflen == BUFSIZE - 1) {
		no_bytes = write_buffer(stream);
		if (no_bytes < 0) {
			stream->err = 1;
			return SO_EOF;
		}
	}

	stream->buffer[stream->buflen] = c;
	stream->buflen++;

	return c;

}

size_t so_fread(void *ptr, size_t size, size_t nmemb, SO_FILE *stream)
{
	if (stream == NULL)
		return SO_EOF;

	int no_bytes = 0;
	size_t scraps_buffer = 0;
	size_t read_size = 0;
	size_t total_bytes = 0;
	size_t copy_size = 0;
	int ret = 0;

	read_size = size * nmemb;
	scraps_buffer = 0;

	if (stream->last_op == WRITE && stream->buflen > 0) {
		ret = so_fflush(stream);
		if (ret != 0)
			return 0;
		lseek(stream->fd, stream->read_offset, SEEK_SET);
	}

	stream->last_op = READ;
	/* if all data from buffer is consumed --> read new data */
	if (stream->nth_ch >= stream->buflen) {
		no_bytes = read_buffer(stream);
		if (no_bytes < 0) {
			stream->err = 1;
			return 0;
		}
		if (no_bytes == 0)
			return 0;
	}

	scraps_buffer = stream->buflen - stream->nth_ch;
	/* if is enough data in the buffer */
	if (scraps_buffer >= read_size) {
		/* copy into specified zone read_size bytes from buffer */
		memcpy(ptr, stream->buffer + stream->nth_ch, read_size);
		stream->nth_ch += read_size;
		stream->read_offset += read_size;
		return read_size / size;
	}

	while (total_bytes < read_size) {
		/* copy the last bytes from buffer in the specified mem zone */
		copy_size = MIN(scraps_buffer, read_size - total_bytes);
		memcpy(ptr + total_bytes,
			stream->buffer + stream->nth_ch,
			copy_size);

		total_bytes += copy_size;
		stream->nth_ch += copy_size;

		if (total_bytes >= read_size)
			break;
		/* read more data in stream buffer */
		no_bytes = read_buffer(stream);
		if (no_bytes < 0) {
			stream->err = 1;
			return 0;
		}
		if (no_bytes == 0) {
			break;
		}

		scraps_buffer = stream->buflen - stream->nth_ch;
	}

	/* return number of elements of this size */
	stream->read_offset += total_bytes;
	return total_bytes / size;
}

size_t so_fwrite(const void *ptr, size_t size, size_t nmemb, SO_FILE *stream)
{
	if (stream == NULL || ptr == NULL)
		return 0;

	int no_bytes = 0;
	size_t write_size = 0;
	size_t total_bytes = 0;
	size_t copy_size = 0;
	size_t free_space_buffer = 0;

	write_size = size * nmemb;
	free_space_buffer = BUFSIZE - stream->buflen;

	/* mark the operation as write */
	stream->last_op = WRITE;
	stream->file_offset = lseek(stream->fd, 0, SEEK_CUR);

	/* if the buffer is full, write the data */
	if (stream->buflen >= BUFSIZE) {
		no_bytes = write_buffer(stream);
		if (no_bytes < 0) {
			stream->err = 1;
			return 0;
		}

		total_bytes += no_bytes;
	}

	if (free_space_buffer >= write_size) {
		memcpy(stream->buffer + stream->buflen, ptr, write_size);
		stream->buflen += write_size;
		if (stream->buflen == BUFSIZE) {
			no_bytes = write_buffer(stream);
			if (no_bytes < 0) {
				stream->err = 1;
				return 0;
			}
			total_bytes += no_bytes;
		}
		stream->file_offset += write_size;
		return write_size / size;
	}

	while (total_bytes < write_size) {
		copy_size = MIN(free_space_buffer, write_size - total_bytes);
		memcpy(stream->buffer + stream->buflen,
			ptr + total_bytes,
			copy_size);

		total_bytes += copy_size;
		stream->buflen += copy_size;

		if (total_bytes >= write_size)
			break;

		// write more data in stream buffer
		if (stream->buflen >= BUFSIZE) {
			no_bytes = write_buffer(stream);
			if (no_bytes < 0) {
				stream->err = 1;
				return 0;
			}
		}

		free_space_buffer = BUFSIZE - stream->buflen;
	}

	stream->file_offset += total_bytes;
	return total_bytes / size;
}

int so_fseek(SO_FILE *stream, long offset, int whence)
{
	int file_offset = 0;

	if (stream->last_op == READ) {
		memset(stream->buffer, 0, BUFSIZE);
		stream->buflen = 0;
		stream->nth_ch = 0;
	} else if (stream->last_op == WRITE)
		so_fflush(stream);

	switch (whence) {
	case SEEK_CUR:
		file_offset = lseek(stream->fd, offset, SEEK_CUR);
		if (file_offset < 0)
			return -1;
		stream->file_offset = file_offset;
		break;

	case SEEK_END:
		file_offset = lseek(stream->fd, offset, SEEK_END);
		if (file_offset < 0)
			return -1;
		stream->file_offset = file_offset;
		break;

	case SEEK_SET:
		file_offset = lseek(stream->fd, offset, SEEK_SET);
		if (file_offset < 0)
			return -1;
		stream->file_offset = file_offset;
		break;

	default:
		return -1;
	}

	return 0;
}

long so_ftell(SO_FILE *stream)
{
	if (stream == NULL)
		return SO_EOF;

	return MAX(stream->file_offset, stream->read_offset);
}

int so_fflush(SO_FILE *stream)
{
	int no_bytes = 0;

	if (stream->last_op == WRITE) {
		no_bytes = write_buffer(stream);
		if (no_bytes < 0) {
			stream->err = 1;
			return SO_EOF;
		}
	}

	return 0;
}

int so_fileno(SO_FILE *stream)
{
	if (stream != NULL)
		return stream->fd;

	return SO_EOF;
}

int so_feof(SO_FILE *stream)
{
	return stream->is_EOF;
}

int so_ferror(SO_FILE *stream)
{
	return stream->err;
}

SO_FILE *so_popen(const char *command, const char *type)
{
	SO_FILE *stream = malloc(sizeof(SO_FILE));
	if (stream == NULL)
		exit(ENOMEM);

	memset(stream, 0, sizeof(SO_FILE));
	return NULL;
}

int so_pclose(SO_FILE *stream)
{
	return 0;
}

