AM_CXXFLAGS = -I$(top_srcdir) -I$(top_srcdir)/src -I$(top_srcdir)/include -I/usr/local/include -pthread
AM_CFLAGS=$(AM_CXXFLAGS)
AM_LDFLAGS= -L/usr/local/lib -lm -lwiringPi

bin_PROGRAMS = 

bin_PROGRAMS+=buttons
buttons_SOURCES=buttons.cpp
buttons_LDADD=$(top_srcdir)/src/.libs/libtumbler.la

bin_PROGRAMS+=buttons2
buttons2_SOURCES=buttons2.cpp
buttons2_LDADD=$(top_srcdir)/src/.libs/libtumbler.la

bin_PROGRAMS+=buttons3
buttons3_SOURCES=buttons3.cpp
buttons3_LDADD=$(top_srcdir)/src/.libs/libtumbler.la

bin_PROGRAMS+=buttons4
buttons4_SOURCES=buttons4.cpp
buttons4_LDADD=$(top_srcdir)/src/.libs/libtumbler.la

bin_PROGRAMS+=ledring
ledring_SOURCES=ledring.cpp
ledring_LDADD=$(top_srcdir)/src/.libs/libtumbler.la

bin_PROGRAMS+=ledring2
ledring2_SOURCES=ledring2.cpp
ledring2_LDADD=$(top_srcdir)/src/.libs/libtumbler.la

bin_PROGRAMS+=versioncheck
versioncheck_SOURCES=versioncheck.cpp
versioncheck_LDADD=$(top_srcdir)/src/.libs/libtumbler.la

if ENVSENSOR
bin_PROGRAMS+=envsensor
envsensor_SOURCES=envsensor.cpp
envsensor_LDADD=$(top_srcdir)/src/.libs/libtumbler.la
endif

if LIGHTSENSOR
bin_PROGRAMS+=lightsensor
lightsensor_SOURCES=lightsensor.cpp
lightsensor_LDADD=$(top_srcdir)/src/.libs/libtumbler.la
endif

if IRIO
AM_LDFLAGS+=-L@PIGPIO_LIBRARY@ -lpigpio
bin_PROGRAMS+=irproximitysensor
irproximitysensor_SOURCES=irproximitysensor.cpp
irproximitysensor_LDADD=$(top_srcdir)/src/.libs/libtumbler.la

bin_PROGRAMS+=irsignalreceiver
irsignalreceiver_SOURCES=irsignalreceiver.cpp
irsignalreceiver_LDADD=$(top_srcdir)/src/.libs/libtumbler.la

bin_PROGRAMS+=irall
irall_SOURCES=irall.cpp
irall_LDADD=$(top_srcdir)/src/.libs/libtumbler.la
endif
