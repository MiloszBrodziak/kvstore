# Key-Value Store
Multithreaded Key-Value Store v0.1

Currently supports a singlethreaded key-value store using a STL unordered_map.

## Build & Run
```bash
cmake -B build && cmake --build build
# Starts server listening on port 4000
./build/kv_server 4000 

# Outputs "SUCCESS!" and sets key "Hello" with value "World!".
./build/kv_client 127.0.0.1 4000 SET Hello World!

# Outputs "$World!".
./build/kv_client 127.0.0.1 4000 GET Hello

# Outputs "SUCCESS!" and deletes key-value pair associated with "Hello".
./build/kv_client 127.0.0.1 4000 DEL Hello

# Outputs "NULL!".
./build/kv_client 127.0.0.1 4000 GET Hello