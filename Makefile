CPPFLAGS=-std=c++2a -static
SRCS=$(wildcard *.cpp)
OBJS=$(SRCS:.cpp=.o)

EBNFformatter: $(OBJS)
	$(CXX) -o EBNFformatter $(OBJS) $(LDFLAGS)

$(OBJS): $(wildcard *.h)

clean:
	rm -f EBNFformatter *.o *~ tmp*

test: EBNFformatter
	./EBNFformatter ../9cc/EBNF.ebnf

.PHONY: test clean
