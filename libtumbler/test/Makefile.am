AM_CXXFLAGS = -I$(top_srcdir)/src -I$(top_srcdir)/include -pthread
AM_LDFLAGS = -L/usr/local/lib -lm -lwiringPi

TESTS = ledring_test
check_PROGRAMS = ledring_test
ledring_test_SOURCES = ledring_test.cpp
ledring_test_LDADD  = $(top_srcdir)/src/tumbler.o
ledring_test_LDADD += $(top_srcdir)/src/ledring.o
