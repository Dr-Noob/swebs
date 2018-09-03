CXX=gcc
SANITY_FLAGS=-Wall -Wextra -Werror -fstack-protector-all -pedantic -Wno-unused -Wfloat-equal -Wshadow -Wpointer-arith -Wstrict-overflow=5 -Wformat=2

MAIN=main.c
WEBSERVER=webserver.c
GET=get.c

HEADERS=webserver.h get.h http.h

OUTPUT=swebs

$(OUTPUT): Makefile $(MAIN) $(WEBSERVER) $(HEADERS)
	$(CXX) $(SANITY_FLAGS) -lm $(MAIN) $(WEBSERVER) $(GET) -o $(OUTPUT)
valgrind:
	valgrind --trace-children=yes ./$(OUTPUT) 8888 .
clean:
	rm $(OUTPUT)
run:
	./$(OUTPUT) 8888 .
