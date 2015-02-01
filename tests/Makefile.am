check_PROGRAMS = gtest
gtest_SOURCES = tests.cpp bptree.cpp deltani.cpp
gtest_LDADD = $(top_srcdir)/src/locations.o $(top_srcdir)/src/util.o

LIBS += $(PTHREAD_LIBS) $(GTEST_LIBS)
AM_CXXFLAGS = $(PTHREAD_CFLAGS)

TESTS = gtest
