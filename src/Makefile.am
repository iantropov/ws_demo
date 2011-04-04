## Process this file with automake to produce Makefile.in

bin_PROGRAMS = demo

demo_SOURCES = 	demo.c

lib_LTLIBRARIES = libdemo.la

libdemo_la_SOURCES = driver.h driver.c util.h util.c log.h log.c

demo_LDADD = libdemo.la