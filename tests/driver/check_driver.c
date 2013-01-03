#include "check.h"
#include <stdlib.h>

#include "../../src/util.h"
#include "../../src/driver.h"

#define DEVICES_FILE "devices_test"
#define DEVICES_JSON "[{\"id\":1, \"info\":\"First device\", \"type\":\"RT\"}, {\"id\":3, \"info\":\"Third device\", \"type\":\"byte\"}]"


START_TEST(test_driver)
{
	fail_unless(put_file_content(DEVICES_FILE, DEVICES_JSON, strlen(DEVICES_JSON) + 1) == 0, "put_file");

	fail_unless(driver_load_devices(DEVICES_FILE) == 0, "driver_init");

	fail_unless(strcmp(driver_get_device_info(1), "First device") == 0, "bad first info");
	fail_unless(driver_get_device_info(2) == NULL, "bad missed info");
	fail_unless(strcmp(driver_get_device_info(3), "Third device") == 0, "bad third info");

	fail_unless(driver_get_device_status(1) == 0, "bad first status");
	fail_unless(driver_get_device_status(2) == -1, "bad second status");
	fail_unless(driver_get_device_status(3) == 0, "bad third status");

	driver_set_device_status(1, 10);
	driver_set_device_status(2, 20);
	driver_set_device_status(3, 30);

	fail_unless(driver_get_device_status(1) == 10, "bad first status");
	fail_unless(driver_get_device_status(2) == -1, "bad second status");
	fail_unless(driver_get_device_status(3) == 30, "bad third status");

	driver_set_device_status(1, -10);
	driver_set_device_status(-2, 20);
	driver_set_device_status(3, 300);

	fail_unless(driver_get_device_status(1) == 10, "bad first status");
	fail_unless(driver_get_device_status(2) == -1, "bad second status");
	fail_unless(driver_get_device_status(3) == 30, "bad third status");

	driver_destroy();
}
END_TEST

TCase *driver_tcase()
{
	TCase *tc_driver = tcase_create ("driver");
	tcase_add_test (tc_driver, test_driver);

	return tc_driver;
}

Suite *make_driver_suite (void)
{
	Suite *s = suite_create ("driver");

	suite_add_tcase (s, driver_tcase());
	
	return s;
}
