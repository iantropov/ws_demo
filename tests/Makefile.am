## Process this file with automake to produce Makefile.in

SUBDIRS= driver util .
TESTS = check_demo
check_PROGRAMS = check_demo
check_demo_SOURCES = 	check_demo.c
check_demo_CFLAGS = @CHECK_CFLAGS@
check_demo_LDADD = 	$(top_builddir)/src/libdemo.la  \
			$(top_builddir)/tests/driver/libcheckdriver.la \
			$(top_builddir)/tests/util/libcheckutil.la \
			 @CHECK_LIBS@