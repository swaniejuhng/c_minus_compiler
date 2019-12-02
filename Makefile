test: main.o util.o lex.yy.o cm.tab.o symtab.o analyze.o cgen.o code.o
	gcc -o project4_24 main.o util.o lex.yy.o cm.tab.o symtab.o analyze.o cgen.o code.o -lfl

main.o: globals.h util.h main.c scan.h lex.yy.c cm.tab.c analyze.h symtab.h
	gcc -c main.c -Wall

util.o: globals.h util.h util.c
	gcc -c util.c -Wall

symtab.o: globals.h symtab.c symtab.h
	gcc -c symtab.c

cgen.o: globals.h cgen.c cgen.h
	gcc -c cgen.c

code.o: globals.h code.c code.h
	gcc -c code.c

analyze.o: analyze.c globals.h symtab.h analyze.h
	gcc -c analyze.c

lex.yy.o: lex.yy.c scan.h util.h globals.h
	gcc -c lex.yy.c

cm.tab.o: cm.tab.c lex.yy.c globals.h
	gcc -c cm.tab.c

lex.yy.c: cm.l globals.h util.h scan.h
	flex cm.l

cm.tab.c: cm.y
	bison -d cm.y

clean :
	rm *.o lex.yy.c cm.tab.c project4_24
