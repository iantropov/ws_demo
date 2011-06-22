/*
 * driver.h
 *
 *  Created on: Mar 21, 2011
 *      Author: ant
 */

#ifndef DRIVER_H_
#define DRIVER_H_

#define DEVICES_LIST_SIZE 255
#define MIN_STATUS_VALUE 0
#define MAX_STATUS_VALUE 255


int driver_load_devices(char *path);
void driver_destroy();

int driver_get_device_status(int id);
char *driver_get_device_info(int id);
char *driver_get_device_type(int id);

void driver_set_device_status(int id, int value);

int driver_start_track(int id, void (*cb)(int id, void *), void *arg, int timeout);
void driver_stop_track(int id);
void driver_stop_track_all();

#endif /* DRIVER_H_ */
