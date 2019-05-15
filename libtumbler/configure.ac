AC_PREREQ([2.69])
AC_INIT([libtumbler],[1.0.0],[contact@fairydevices.jp])
AC_CONFIG_SRCDIR([src/tumbler.cpp])
AC_CONFIG_HEADERS([src/config.h])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
m4_include([m4/pkg.m4])
m4_pattern_allow([AM_PROG_AR])
AM_PROG_AR

#------------------------------------------------------------------------------------               
# Rules for library version information:                                
# 
#  1. Start with version information of `0:0:0' for each libtool library. 
#  2. Update the version information only immediately before a public release of
#     your software. More frequent updates are unnecessary, and only guarantee
#     that the current interface number gets larger faster.
#  3. If the library source code has changed at all since the last update, then
#     increment revision (`c:r:a' becomes `c:r+1:a').                                    
#  4. If any interfaces have been added, removed, or changed since the last update,
#     increment current, and set revision to 0.
#  5. If any interfaces have been added since the last public release, then increment
#     age.
#  6. If any interfaces have been removed since the last public release, then set age
#     to 0.
SHARED_VERSION_INFO="1:0:0"

# Save User CFLAGS
CFLAGS_SAVE=$CFLAGS
CXXFLAGS_SAVE=$CXXFLAGS

AC_PROG_CXX
AC_PROG_CC
AC_CANONICAL_HOST
AC_LANG([C++])
if test "$CXX" = no || test "$CXX:$GXX" = "g++:"; then
  AC_MSG_ERROR([no C++ compiler found])
fi
AC_PROG_LIBTOOL
AC_PROG_INSTALL
AC_PROG_MAKE_SET

# Debug build option
AC_ARG_ENABLE(debug,
AS_HELP_STRING([--enable-debug],
               [Enable debugging, default: no]),
	       [case "${enableval}" in
	       	     yes) debug=true ;;
		     no)  debug=false ;;
		     *)   AC_MSG_ERROR([invalid value ${enableval} for --enable-debug]) ;;
		esac],
		[debug=false])
AM_CONDITIONAL(DEBUG, test x"$debug" = x"true")

# Restore (Redefine) CFLAGS to avoid '-g -O2' introduced by AC_PROG_CC/CXX
if test x"$debug" = x"true"; then
   CFLAGS="$CFLAGS_SAVE -g -O0 -Wall"
   CXXFLAGS="$CXXFLAGS_SAVE -g -O0 -Wall"
else
   CFLAGS="$CFLAGS_SAVE -O3 -DNDEBUG -Wall -fvisibility=hidden"
   CXXFLAGS="$CXXFLAGS_SAVE -O3 -DNDEBUG -Wall -fvisibility=hidden -fvisibility-inlines-hidden"
fi

## Environmental sensor support
AC_ARG_ENABLE(envsensor,
AS_HELP_STRING([--enable-envsensor],
               [Enable environmental sensor (bme280), default: no]),
	       [case "${enableval}" in
	       	     yes) envsensor=true ;;
		     no)  envsensor=false ;;
		     *)   AC_MSG_ERROR([invalid value ${enableval} for --enable-envsensor]) ;;
		esac],
		[envsensor=false])
AM_CONDITIONAL(ENVSENSOR, test x"$envsensor" = x"true")

AC_SUBST(CFLAGS)
AC_SUBST(CXXFLAGS)
AC_SUBST(SHLIB_VERSION_ARG)
AC_SUBST(SHARED_VERSION_INFO)
AC_SUBST(OS_SPECIFIC_INCLUDES)
AC_SUBST(OS_SPECIFIC_LINKS)
AC_SUBST(ENVSENSOR)

# Check for C++11
AX_CXX_COMPILE_STDCXX_11([ext],[mandatory])

AC_CONFIG_FILES([Makefile
				 m4/Makefile
				 doc/Makefile
				 test/Makefile
				 include/Makefile
				 include/tumbler/Makefile
                 examples/Makefile
                 src/Makefile
                 src/tumbler.pc])
AC_OUTPUT

AC_MSG_RESULT([
-=-=-=-=-=-=-=-=-=-= Configuration Complete =-=-=-=-=-=-=-=-=-=-=-

  Configuration summary :

    Version : ..................... ${VERSION}

    Host CPU : .................... ${host_cpu}
    Host Vendor : ................. ${host_vendor}
    Host OS : ..................... ${host_os}
])

AC_MSG_RESULT([
    CC: ........................... ${CC}
    CXX: .......................... ${CXX}
    CFLAGS: ....................... ${CFLAGS}
    CXXFLAGS: ..................... ${CXXFLAGS} 
])

if test x"$envsensor" = x"true";then
AC_MSG_RESULT([
    Environmetanl sensor support .. yes
])
else
AC_MSG_RESULT([
    Environmetanl sensor support .. no
])
fi

if test x"$debug" = x"true";then
AC_MSG_RESULT([
  Debug build ..................... yes
])
else
AC_MSG_RESULT([
    Debug build ................... no
])
fi

AC_MSG_RESULT([  Installation directories :

    Library directory : ........... ${prefix}/lib
    Program directory : ........... ${prefix}/bin
    Pkgconfig directory : ......... ${prefix}/lib/pkgconfig
])
