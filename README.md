# Key-Value Store
Multithreaded Key-Value Store v0.1

Currently supports a singlethreaded key-value store using a STL unordered_map.

## Build & Run
```bash
cmake -B build && cmake --build build
./build/kv_server                   # Starts server
./build/kv_client SET Hello World!  # Outputs "SUCCESS!" and sets key "Hello" with value "World!".
./build/kv_client GET Hello         # Outputs "$World!".
./build/kv_client DEL Hello         # Outputs "SUCCESS!" and deletes key-value pair associated with "Hello".
./build/kv_client GET Hello         # Outputs "NULL!".