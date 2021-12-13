## Run autoconf on this file to produce configure.in
##=============================================================================
##
## mister: configure.ac:
##
##=============================================================================


#-----------------------------
#           Init:
#-----------------------------

# NOTE: This is the PACKAGE release version, NOT the ABI version. It is SemVer:
AC_INIT([mister],[0.0.0],[mlaing@post.harvard.edu],[mister],[https://github.com/michaelplaing/MisteR])

# More package metadata:
AC_SUBST([PACKAGE_LICENSE],["MIT"])
AC_SUBST([PACKAGE_VENDOR],["Michael P Laing"])
AC_SUBST([PACKAGE_COPYRIGHT],
    ["© copyright 2021 Michael P. Laing"])
AC_COPYRIGHT([© copyright 2021 Michael P. Laing])
AC_SUBST([PACKAGE_DESCRIPTION],["Redis module and interface library for MQTT protocol"])
AC_SUBST([PACKAGE_SUMMARY],["MQTT module for Redis"])

# HACK: get the version components (note: we get awk early here)
AC_PROG_AWK
AC_SUBST([PACKAGE_MAJOR],[`echo $PACKAGE_VERSION | awk -F. '{print $1}'`])
AC_SUBST([PACKAGE_MINOR],[`echo $PACKAGE_VERSION | awk -F. '{print $2}'`])
AC_SUBST([PACKAGE_PATCH],[`echo $PACKAGE_VERSION | awk -F. '{print $3}'`])
AC_SUBST([MISTER_VERSION],[`echo $PACKAGE_VERSION | awk -F. '{printf("0x%02x%02x%02x00", $1, $2, $3)}'`])

# Tell autoconf where to find stuff:
AC_CONFIG_AUX_DIR([.])
AC_CONFIG_MACRO_DIR([m4])

#-----------------------------
#         Programs:
#-----------------------------
AM_INIT_AUTOMAKE([-Wall foreign])
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_MAKE_SET
m4_ifdef([AM_PROG_AR], [AM_PROG_AR])
LT_INIT
PKG_PROG_PKG_CONFIG()

#-----------------------------
#   Make sure we have libev:
#-----------------------------
AC_CHECK_LIB([ev],[ev_version_major],[],[
    AC_MSG_ERROR([libev is required to build mr])
])

AC_CHECK_HEADERS([ev.h],[],[
    AC_MSG_ERROR([libev is required to build mr])
])

#-----------------------------
#           Types:
#-----------------------------
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T

#-----------------------------
#         Features:
#-----------------------------
AC_ARG_ENABLE([maintainer-debug],
    AS_HELP_STRING(
        [--enable-maintainer-debug],
        [Stricter compile-time rules and increased verbosity for maintainers]))

# If maintainer debug is set, get picky and be verbose:
AS_IF([test "x$enable_maintainer_debug" = "xyes"], [
    AC_MSG_NOTICE([Maintainer debug enabled])
    CFLAGS="${CFLAGS} -O0 -g -Wall -std=c99 -Werror "
    ])
AM_CONDITIONAL([MAINTAINER_DEBUG],[test "x$enable_maintainer_debug" = "xyes"])

#-----------------------------
#          Output:
#-----------------------------
AC_CONFIG_HEADERS([mister_config.h])
AC_CONFIG_FILES(
  include/connect.h
  include/mister.h
  include/pack.h
  include/redismodule.h
  Makefile
  module/Makefile
  include/Makefile
  test/Makefile
  mister.pc
)
AC_OUTPUT
AS_BOX([mister configuration complete])
