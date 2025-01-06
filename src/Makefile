build: tema2.o upload.o tracker.o download.o tags.o
	mpicxx -o tema2 tema2.o upload.o tracker.o download.o tags.o -pthread -Wall

tema2.o: tema2.cpp
	mpicxx -o tema2.o -c tema2.cpp -pthread -Wall

upload.o: upload.cpp
	mpicxx -o upload.o -c upload.cpp -pthread -Wall

tracker.o: tracker.cpp
	mpicxx -o tracker.o -c tracker.cpp -pthread -Wall

download.o: download.cpp
	mpicxx -o download.o -c download.cpp -pthread -Wall

tags.o: tags.cpp
	mpicxx -o tags.o -c tags.cpp -pthread -Wall
	
clean:
	rm tema2 tema2.o upload.o
