EXTRA_DIST = config.h tumbler.pc.in
AM_CXXFLAGS = -I$(top_srcdir)/src -I$(top_srcdir)/include
OS_SPECIFIC_CFLAGS = @OS_SPECIFIC_CFLAGS@
OS_SPECIFIC_LINKS = @OS_SPECIFIC_LINKS@
lib_LTLIBRARIES = libtumbler.la
pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = tumbler.pc
libtumbler_la_LDFLAGS = -L/usr/local/lib -no-undefined -version-info @SHARED_VERSION_INFO@ @SHLIB_VERSION_ARG@
libtumbler_la_LIBADD = -lm
libtumbler_la_SOURCES = tumbler.cpp ledring.cpp speaker.cpp

