#include "check.h"

#include "../../src/util.h"

#include <stdlib.h>
#include <string.h>


#define TEST_FILE "file"

START_TEST(test_string_copy)
{
	char *s = "String1";
	char *s_copy = string_copy(s);

	fail_unless(strcmp(s, s_copy) == 0, "string_copy");
}
END_TEST

START_TEST(test_string_ends_by)
{
	fail_unless(string_ends_by("file.css", ".css") == 1, "string_ends_by");
	fail_unless(string_ends_by("filecss", ".css") == 0, "string_ends_by");
}
END_TEST

START_TEST(test_file_contents)
{
	char *s = "String1";
	fail_unless(put_file_content(TEST_FILE, s, strlen(s) + 1) == 0, "put_file");

	char *s_from_file = get_file_content(TEST_FILE);
	fail_unless(s_from_file != NULL, "get_file");
	fail_unless(strcmp(s_from_file, s) == 0, "string original and string from file not equals");

	free(s_from_file);
}
END_TEST


TCase *util_tcase()
{
	TCase *tc_util = tcase_create ("util tcase");
	tcase_add_test (tc_util, test_string_copy);
	tcase_add_test (tc_util, test_file_contents);
	tcase_add_test (tc_util, test_string_ends_by);

	return tc_util;
}

Suite *make_util_suite (void)
{
	Suite *s = suite_create ("util suite");

	suite_add_tcase (s, util_tcase());
	
	return s;
}
