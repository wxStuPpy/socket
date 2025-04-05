CXX=g++

CXXFLAGS=-Wall  -std=c++11

dup:dup.cpp
	-${CXX} $^ ${CXXFLAGS} -o $@
	-./$@ 0.0.0.0 12345
	-rm ./$@

pipe:pipe.cpp
	-${CXX} $^ ${CXXFLAGS} -o $@
	-./$@
	-rm ./$@

readv_writev:readv_writev.cpp
	-${CXX} $^ ${CXXFLAGS} -o $@
	-./$@ 127.0.0.1 12345 test.txt
	-rm ./$@

sendfile:sendfile.cpp
	-${CXX} $^ ${CXXFLAGS} -o $@
	-./$@ 127.0.0.1 12345 test.txt
	-rm ./$@

splice:splice.cpp
	-${CXX} $^ ${CXXFLAGS} -o $@
	-./$@ 0.0.0.0 12345
	-rm ./$@