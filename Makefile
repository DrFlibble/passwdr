
SRCS=passwdr.cpp sha1.cpp
OBJS=$(SRCS:.cpp=.o)

all: $(OBJS)
	gcc $(OBJS) -o passwdr -lstdc++

.cpp.o:
	gcc -c $< -o $@

