AM_CXXFLAGS = -I$(top_srcdir)/src -I$(top_srcdir)/include -pthread
AM_LDFLAGS = -L/usr/local/lib -lm -lwiringPi

TESTS = ledring_test
check_PROGRAMS = ledring_test
ledring_test_SOURCES = ledring_test.cpp
ledring_test_LDADD  = $(top_srcdir)/src/tumbler.o
ledring_test_LDADD += $(top_srcdir)/src/ledring.o

TESTS += speaker_test
check_PROGRAMS += speaker_test
speaker_test_SOURCES = speaker_test.cpp
speaker_test_LDADD  = $(top_srcdir)/src/tumbler.o
speaker_test_LDADD += $(top_srcdir)/src/speaker.o -lasound

TESTS += buttons_test
check_PROGRAMS += buttons_test
buttons_test_SOURCES = buttons_test.cpp
buttons_test_LDADD  = $(top_srcdir)/src/tumbler.o
buttons_test_LDADD += $(top_srcdir)/src/buttons.o -lasound
buttons_test_LDADD += $(top_srcdir)/src/speaker.o -lasound
