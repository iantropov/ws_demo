/*
 * driver.h
 *
 *  Created on: Mar 21, 2011
 *      Author: ant
 */

#ifndef DRIVER_H_
#define DRIVER_H_

int driver_init();
int driver_get_device_status(int id);
char *driver_get_device_info(int id);
void driver_set_device_status(int id, int value);
void driver_destroy();

#endif /* DRIVER_H_ */
