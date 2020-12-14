all:
	gcc -g -fsanitize=address -std=c11 -Wall -Werror first.c -o first -lm 
clean:
	rm -f first
