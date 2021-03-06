CXX=gcc
SANITY_FLAGS=-Wall -Wextra -Werror -fstack-protector-all -pedantic -Wno-unused -Wfloat-equal -Wshadow -Wpointer-arith -Wstrict-overflow=5 -Wformat=2

MAIN=main.c
WEBSERVER=webserver.c
GET=get.c
LOG=log.c
ARGS=args.c

HEADERS=webserver.h get.h http.h log.h args.h

OUTPUT=swebs

$(OUTPUT): Makefile $(MAIN) $(WEBSERVER) $(GET) $(LOG) $(ARGS) $(HEADERS)
	$(CXX) $(SANITY_FLAGS) -g -lm -pthread $(MAIN) $(WEBSERVER) $(GET) $(LOG) $(ARGS) -o $(OUTPUT)
clean:
	rm $(OUTPUT)
