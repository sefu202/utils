test_fifo: fifo_test.o fifo.c
	gcc fifo_test.o fifo.o -o test_fifo

fifo_test.o: fifo_test.c
	gcc -c fifo_test.c

fifo.o: fifo.c
	gcc -c fifo.c

clean_windows: 
	del *.o test_fifo.exe

clean:
	rm *.o test_fifo