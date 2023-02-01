CC=gcc

proxy: proxy.o simpleSocketAPI.o
	${CC} $^ -o $@ 
proxy.o: proxy.c simpleSocketAPI.h
	${CC} -c $^ 
simpleSocketAPI.o: simpleSocketAPI.c
	${CC} -c $^ 
client:
	${CC} -o client client.c
clean:
	rm -f *.o proxy client