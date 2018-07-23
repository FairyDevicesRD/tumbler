AM_CXXFLAGS = -I$(top_srcdir)/src -I$(top_srcdir)/include -pthread
AM_LDFLAGS = -L/usr/local/lib -lm -lwiringPi

TESTS = ledring_test
check_PROGRAMS = ledring_test
ledring_test_SOURCES = ledring_test.cpp
ledring_test_LDADD  = $(top_srcdir)/src/tumbler.o
ledring_test_LDADD += $(top_srcdir)/src/ledring.o
ledring_test_LDADD += $(top_srcdir)/src/touch.o
ledring_test_LDADD += $(top_srcdir)/src/bme.o

TESTS += touch_test
check_PROGRAMS += touch_test
touch_test_SOURCES = touch_test.cpp
touch_test_LDADD  = $(top_srcdir)/src/tumbler.o
touch_test_LDADD += $(top_srcdir)/src/ledring.o
touch_test_LDADD += $(top_srcdir)/src/touch.o
touch_test_LDADD += $(top_srcdir)/src/bme.o

TESTS += speaker_test
check_PROGRAMS += speaker_test
speaker_test_SOURCES = speaker_test.cpp
speaker_test_LDADD  = $(top_srcdir)/src/tumbler.o
speaker_test_LDADD += $(top_srcdir)/src/touch.o
speaker_test_LDADD += $(top_srcdir)/src/bme.o
speaker_test_LDADD += $(top_srcdir)/src/speaker.o -lasound

TESTS += serial_test
check_PROGRAMS += serial_test
serial_test_SOURCES = serial_test.cpp
serial_test_LDADD  = $(top_srcdir)/src/tumbler.o
serial_test_LDADD += $(top_srcdir)/src/ledring.o
serial_test_LDADD += $(top_srcdir)/src/touch.o
serial_test_LDADD += $(top_srcdir)/src/bme.o

TESTS += bme_test
check_PROGRAMS += bme_test
bme_test_SOURCES = bme_test.cpp
bme_test_LDADD  = $(top_srcdir)/src/tumbler.o
bme_test_LDADD += $(top_srcdir)/src/touch.o
bme_test_LDADD += $(top_srcdir)/src/bme.o


