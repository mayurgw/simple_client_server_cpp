## simple client server model

### compilation and running server
g++ -std=c++11 -pthread 183050039_server.cpp -o server__
./server 127.0.0.1 3000

### compilation and running client
g++ -std=c++11 183050039_client.cpp -o client__
./client interactive__
./client batch filepath

### Note
This is an example of multithreaded server architecture
