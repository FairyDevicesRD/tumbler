AM_CXXFLAGS = -I$(top_srcdir) -I$(top_srcdir)/src -I$(top_srcdir)/include -I/usr/local/include -pthread
AM_CFLAGS=$(AM_CXXFLAGS)
AM_LDFLAGS= -L/usr/local/lib -lm -lwiringPi

bin_PROGRAMS = 

bin_PROGRAMS+=buttons
buttons_SOURCES=buttons.cpp
buttons_LDADD=$(top_srcdir)/src/.libs/libtumbler.la
