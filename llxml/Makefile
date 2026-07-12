CXX = clang++
CXXFLAGS = -std=c++17 -I../llcommon

# define the C++ source files
SRCS = llxml.cpp fileutil.cpp xml.cpp ../llcommon/directory.cpp

OBJS = $(SRCS:.cpp=.o)

# define the executable file
MAIN = llxml

all: $(MAIN)


$(MAIN): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(MAIN) $(OBJS) # $(LFLAGS) $(LIBS)


clean:
	rm -rf *.o* ../llcommon/*.o $(MAIN)


#depend: $(SRCS)
#    makedepend $(INCLUDES) $^

# DO NOT DELETE THIS LINE -- make depend needs it
