#
# FF_ROOT     points to the FastFlow root directory (i.e.
#             the one containing the ff directory).
ifndef FF_ROOT
FF_ROOT		= ../../Lib/FastFlow
endif

CXX		= g++ -std=c++17 #-DNO_DEFAULT_MAPPING
INCLUDES	= -I $(FF_ROOT) -I ./
CXXFLAGS  	= -g

LDFLAGS 	= -pthread
OPTFLAGS	= -O3 -finline-functions -DNDEBUG -g

TARGETS		= ./Test/mainFib

.PHONY: all clean cleanall
.SUFFIXES: .cpp


%: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(OPTFLAGS) -o $@.out $< $(LDFLAGS)

all		: $(TARGETS)
clean		:
	rm -f $(TARGETS).out
cleanall	: clean
	\rm -f *.o *~

test:
	./Test/mainFib.out 4 1000 30 33 35 36
	./Test/mainPrime.out 4 1000 1000000 15487469 1301081 15485867
	./Test/mainPalindrome.out 4 100 1000000 "DennisNellEdnaLeonNedraAnitaRolfNoraAliceCarolLeoJaneReedDenaDaleBasilRaePennyLanaDaveDennyLenaIdaBernadetteBenRayLilaNinaJoIraMaraSaraMarioJanInaLilyArneBetteDanRebaDianeLynnEdEvaDanaLynnePearlIsabelAdaNedDeeRenaJoelLoraCecilAaronFloraTinaArdenNoelandEllensinned" "itopinonavevanonipoti" "ciaomondo"
