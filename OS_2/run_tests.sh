gcc -lpthread server.c -o server

gcc client.c -o client

./server 

./client near 1 1 1 &

./cleint far 2 1 0 &
./cleint near 0 0 3 &
./client far 1 2 0 &
./cleint far 2 1 0 &
./cleint near 0 0 3 &
./client far 1 2 0 &
./cleint far 2 1 0 &
./cleint near 0 0 3 &
./client far 1 2 0 &
./client near 0 1 0 &