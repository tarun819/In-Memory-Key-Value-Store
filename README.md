# In-Memory-Key-Value-Store
A from-scratch in-memory key-value store, similar to the core of Redis, built in C++. It is a TCP server: clients connect over the network and send text commands like `SET key value` or `GET key`. The server stores data in RAM and responds in microseconds.  
