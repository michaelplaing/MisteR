##=============================================================================
##
## mr: test/Makefile.am
##
##=============================================================================

bin_PROGRAMS = mr

mr_SOURCES = mr.c

AM_CFLAGS= -I@top_builddir@/include -g

AM_LDFLAGS= -lev -lhiredis -luuid

doc: mr
	pomd4c @srcdir@/mr.c > @srcdir@/README.md

# EOF
