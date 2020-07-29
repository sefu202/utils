test_fifo: fifo_test.o fifo.o
	gcc fifo_test.o fifo.o test.o -o test_fifo

fifo_test.o: fifo_test.c
	gcc -c fifo_test.c

test.o: test.c
	gcc -c test.c

fifo.o: fifo.c
	gcc -c fifo.c

clean_windows: 
	del *.o *.exe

clean:
	rm *.o test_fifo