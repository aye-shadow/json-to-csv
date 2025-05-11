CC=gcc
LEX=flex
YACC=bison

CFLAGS=-Wall -g

# Default target
all: json2relcsv

# Generate parser.c and parser.h from parser.y
parser.c parser.h: parser.y
	$(YACC) -d -o parser.c parser.y

# Generate scanner.c from scanner.l, which needs parser.h
scanner.c: scanner.l parser.h
	$(LEX) -o scanner.c scanner.l

# Compile object files
scanner.o: scanner.c
	$(CC) $(CFLAGS) -c scanner.c

parser.o: parser.c
	$(CC) $(CFLAGS) -c parser.c

ast.o: ast.c
	$(CC) $(CFLAGS) -c ast.c

schema.o: schema.c
	$(CC) $(CFLAGS) -c schema.c

walker.o: walker.c
	$(CC) $(CFLAGS) -c walker.c

main.o: main.c
	$(CC) $(CFLAGS) -c main.c

# Final link
json2relcsv: scanner.o parser.o ast.o schema.o walker.o main.o
	$(CC) $(CFLAGS) -o json2relcsv $^

# Clean build files
clean:
	rm -f *.o *.csv scanner.c parser.c parser.h json2relcsv
