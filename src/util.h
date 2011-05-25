/*
 * resource_handler.h
 *
 *  Created on: Mar 21, 2011
 *      Author: ant
 */

#ifndef RESOURCE_HANDLER_H_
#define RESOURCE_HANDLER_H_

void *get_file_content(char *path);
int get_file_size(char *path);

int put_file_content(char *path, void *data, int size);

int string_ends_by(char *source, char *end);

#endif /* RESOURCE_HANDLER_H_ */
