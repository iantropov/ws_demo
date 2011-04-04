/*
 * resource_handler.c
 *
 *  Created on: Mar 21, 2011
 *      Author: ant
 */

#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

#include "log.h"

int get_file_size(char *path)
{
	struct stat stat_uri;
	if (stat(path, &stat_uri) == -1) {
		return -1;
	}

	return stat_uri.st_size;
}

static void *read_file(int fd, int length)
{
	void *buf = malloc(length);
	if (buf == NULL) {
		log_warn("%s : malloc failed\n", __func__);
		return NULL;
	}

	void *origin_buf = buf;
	int ret;
	while (length > 0 && (ret = read(fd, buf, length)) != 0) {
		if (ret == -1) {
			if (errno == EINTR)
				continue;

			log_warn("%s : read failed\n", __func__);
			goto error;
		}

		length -= ret;
		buf += ret;
	}

	if (length > 0)
		goto error;

	return origin_buf;

error:
	free(origin_buf);
	return NULL;
}

void *get_file_content(char *path)
{
	int len_file = get_file_size(path);
	if (len_file == -1) {
		log_warn("stat : can`t find %s\n", path);
		return NULL;
	}

	int fd = open(path, O_RDONLY);
	if (fd == -1) {
		log_warn("open : can`t open %s\n", path);
		return NULL;
	}

	void *buf = read_file(fd, len_file);

	close(fd);

	return buf;
}

static int write_file(int fd, void *data, int len)
{
	int ret;
	while (len > 0 && (ret = write(fd, data, len)) != 0) {
		if (ret == -1) {
			if (errno == EINTR)
				continue;

			log_warn("%s : write failed\n", __func__);
			break;
		}

		len -= ret;
		data += ret;
	}

	return len > 0 ? -1 : 0;
}

int put_file_content(char *path, void *data, int size)
{
	int fd = open(path, O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR |
														S_IRGRP | S_IWGRP |
														S_IROTH | S_IWOTH);
	if (fd == -1) {
		log_warn("open : failed open : %s\n", path);
		return -1;
	}

	int ret = write_file(fd, data, size);

	close(fd);

	return ret;
}

char *string_copy(char *source)
{
	if (source == NULL)
		return NULL;

	int len = strlen(source);
	char *dest = (char *)malloc(len + 1);
	if (dest == NULL) {
		log_warn("%s : malloc failed\n", __func__);
		return NULL;
	}
	return strcpy(dest, source);
}

int string_ends_by(char *str, char *end)
{
	char *loc = strstr(str, end);
	return (loc != NULL && strlen(loc) == strlen(end));
}
