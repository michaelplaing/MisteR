##=============================================================================
##
## mr: test/Makefile.am
##
##=============================================================================

bin_PROGRAMS = mr

mr_SOURCES = mr.c pack.c

AM_CFLAGS= -I@top_builddir@/include -g

AM_LDFLAGS= -lev -lhiredis -luuid -lJudy

doc: mr
	pomd4c @srcdir@/mr.c > @srcdir@/README.md

# EOF
