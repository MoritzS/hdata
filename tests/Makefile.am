check_PROGRAMS = gtest
gtest_SOURCES = tests.cpp bptree.cpp
gtest_LDADD = /usr/src/gtest/libgtest.a

LIBS += $(PTHREAD_LIBS)
AM_CXXFLAGS = $(PTHREAD_CFLAGS)

TESTS = gtest
