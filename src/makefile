#
# FF_ROOT     points to the FastFlow root directory (i.e.
#             the one containing the ff directory).
ifndef FF_ROOT
FF_ROOT		= /Users/lucacorbucci/Dropbox/Università/Magistrale/"Parallel and distributed systems: paradigms and models"/SPM19/FastFlow
endif

CXX		= g++ -std=c++17 #-DNO_DEFAULT_MAPPING
INCLUDES	= -I $(FF_ROOT)
CXXFLAGS  	= -g

LDFLAGS 	= -pthread
OPTFLAGS	= -O3 -finline-functions -DNDEBUG

TARGETS		= main

.PHONY: all clean cleanall
.SUFFIXES: .cpp


%: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(OPTFLAGS) -o $@ $< $(LDFLAGS)

all		: $(TARGETS)
clean		:
	rm -f $(TARGETS)
cleanall	: clean
	\rm -f *.o *~