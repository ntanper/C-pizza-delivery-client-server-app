gcc -lrt server.c -o server
gcc client.c -o client
./server &
./client near 1 1 1 &

