build: tema2.o upload.o tracker.o
	mpicxx -o tema2 tema2.o upload.o tracker.o -pthread -Wall

tema2.o: tema2.cpp
	mpicxx -o tema2.o -c tema2.cpp -pthread -Wall

upload.o: upload.cpp
	mpicxx -o upload.o -c upload.cpp -pthread -Wall

tracker.o: tracker.cpp
	mpicxx -o tracker.o -c tracker.cpp -pthread -Wall

clean:
	rm tema2 tema2.o upload.o
