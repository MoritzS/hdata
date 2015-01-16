check_PROGRAMS = gtest
gtest_SOURCES = tests.cpp bptree.cpp deltani.cpp
gtest_LDADD = /usr/src/gtest/libgtest.a $(top_srcdir)/src/locations.o $(top_srcdir)/src/util.o

LIBS += $(PTHREAD_LIBS)
AM_CXXFLAGS = $(PTHREAD_CFLAGS)

TESTS = gtest
