.SUFFIXES: .cpp .o
SHELL=/bin/sh
CXX=g++
DEBUG= -g
OPT= -O2 -DNDEBUG
CXXFLAGS= -c $(DEBUG) $(OPT) -std=c++0x
LDFLAGS= 


LIBS = -lstdc++
OBJS = fp.o sa.o
B_OBJS  = btree.o main.o $(OBJS)
SRCS = ${OBJS:%.o=%.cpp}

all:    floorplan 

floorplan: $(B_OBJS)
	$(CXX) -o floorplan $(B_OBJS) $(LIBS) $(LDFLAGS)

%.o : %.cpp %.h fp.h btree.h
	$(CXX) $*.cpp $(CXXFLAGS)

%.o : %.cpp  fp.h btree.h
	$(CXX) $*.cpp $(CXXFLAGS)

clean: 
	rm -f *.o btree *~

compact : btree
	strip $?

