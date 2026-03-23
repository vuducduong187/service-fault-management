all:
	gcc -o Pm_1 test1.c
	gcc -o Pm_2 test2.c
	gcc -o sfmd sfmd.c
clean:
	rm -f Pm_1 Pm_2 sfmd
