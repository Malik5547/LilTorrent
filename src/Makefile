build: tema2.o upload.o
	mpicxx -o tema2 tema2.o upload.o -pthread -Wall

tema2.o: tema2.cpp
	mpicxx -o tema2.o -c tema2.cpp -pthread -Wall

upload.o: upload.cpp
	mpicxx -o upload.o -c upload.cpp -pthread -Wall

clean:
	rm tema2 tema2.o upload.o
