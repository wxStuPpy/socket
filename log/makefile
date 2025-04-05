CXX=g++

CXXFLAGS=-Wall  -std=c++11

syslog:syslog.cpp
	-${CXX} $^ ${CXXFLAGS} -o $@
	-./$@ 
	-rm ./$@
