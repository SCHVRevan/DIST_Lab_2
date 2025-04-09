#include <iostream>
#include <string>
#include <memory>
#include <iomanip>
#include "Allocator.h"
#include <assert.h>
#include <new>
#include <chrono>
using namespace std;
using namespace std::chrono;

class Person {
    DECLARE_ALLOCATOR
    string first_name;
    string second_name;
    int age;
    public:
        Person(string f = "null", string s = "null", int a = 0) {
            first_name = f;
            second_name = s;
            age = a;
        }
        void set_f_name(string f) {first_name = f;}
        void set_s_name(string s) {second_name = s;}
        void set_age(int a) {age = a;}
        string get_f_name() {return first_name;}
        string get_s_name() {return second_name;}
        int get_age() {return age;}
};
IMPLEMENT_ALLOCATOR(Person, 0, 0)

// Heap blocks mode unlimited with 100 byte blocks
Allocator allocHeapBlocks(100);

// Heap pool mode with 20, 100 byte blocks
Allocator allocHeapPool(100, 20);

// Static pool mode with 20, 100 byte blocks
char staticMemoryPool[100 * 20];
Allocator allocStaticPool(100, 20, staticMemoryPool);

// Static pool mode with 20 MyClass sized blocks using template
AllocatorPool<Person, 20> allocStaticPool2;

// Benchmark allocators
static const int MAX_BLOCKS = 10000;
static const int MAX_BLOCK_SIZE = 4096;
void* memoryPtrs[MAX_BLOCKS];
void* memoryPtrs2[MAX_BLOCKS];
AllocatorPool<char[MAX_BLOCK_SIZE], MAX_BLOCKS*2> allocatorStaticPoolBenchmark;
Allocator allocatorHeapBlocksBenchmark(MAX_BLOCK_SIZE);

static void out_of_memory() {
	// new-handler function called by Allocator when pool is out of memory
    cout << "Out of memory!\n";
	assert(0);
}

typedef void* (*AllocFunc)(int size);
typedef void (*DeallocFunc)(void* ptr);
void Benchmark(const char* name, AllocFunc allocFunc, DeallocFunc deallocFunc);
void* AllocHeap(int size);
void DeallocHeap(void* ptr);
void* AllocStaticPool(int size);
void DeallocStaticPool(void* ptr);
void* AllocHeapBlocks(int size);
void DeallocHeapBlocks(void* ptr);

int main() {
    set_new_handler(out_of_memory); // !!!

    Person* some_person = new Person();
    delete some_person;
    
    void* test_mem[50];
    // HeapBlocks
    for (int i = 0; i < 20; i++) {
        test_mem[i] = allocHeapBlocks.Allocate(100);
        cout << "allocHeapBlocks: " << allocHeapBlocks.GetBlockCount() << "\n";
    }
    for (int i = 0; i < 20; i++) {allocHeapBlocks.Deallocate(test_mem[i]);}

    // HeapPool
    cout << "\n   HeapPool size: " << allocHeapPool.GetBlockCount() << "\n";
    for (int i = 0; i < 20; i++) {
        test_mem[i] = allocHeapPool.Allocate(100);
        cout << "allocHeapPool: " << allocHeapPool.GetBlocksInUse() << "\n";
    }
    for (int i = 0; i < 20; i++) {
        allocHeapPool.Deallocate(test_mem[i]);\
        cout << "allocHeapPool dealloc: " << allocHeapPool.GetBlocksInUse() << "\n";
    }

    // StaticPool
    cout << "\n   StaticPool size: " << allocStaticPool.GetBlockCount() << "\n";
    for (int i = 0; i < 21; i++) {
        test_mem[i] = allocStaticPool.Allocate(100);
        cout << "allocStaticPool: " << allocStaticPool.GetBlocksInUse() << "\n";
    }
    for (int i = 0; i < 20; i++) {allocStaticPool.Deallocate(test_mem[i]);}
    

    void* memo1 = allocHeapBlocks.Allocate(100);
    allocHeapBlocks.Deallocate(memo1);

    void* memo2 = allocHeapBlocks.Allocate(100);
    allocHeapBlocks.Deallocate(memo2);

    void* memo3 = allocHeapPool.Allocate(100);
    allocHeapPool.Deallocate(memo3);
    void* memo4 = allocHeapPool.Allocate(100);
    allocHeapPool.Deallocate(memo4);

    void* memo5 = allocStaticPool.Allocate(100);
    allocStaticPool.Deallocate(memo5);

    void* memo6 = allocStaticPool2.Allocate(sizeof(Person));
    allocStaticPool2.Deallocate(memo6);
/*
    Benchmark("Heap (Run 1)", AllocHeap, DeallocHeap);
	Benchmark("Heap (Run 2)", AllocHeap, DeallocHeap);
	Benchmark("Heap (Run 3)", AllocHeap, DeallocHeap);
	Benchmark("Static Pool (Run 1)", AllocStaticPool, DeallocStaticPool);
	Benchmark("Static Pool (Run 2)", AllocStaticPool, DeallocStaticPool);
	Benchmark("Static Pool (Run 3)", AllocStaticPool, DeallocStaticPool);
	Benchmark("Heap Blocks (Run 1)", AllocHeapBlocks, DeallocHeapBlocks);
	Benchmark("Heap Blocks (Run 2)", AllocHeapBlocks, DeallocHeapBlocks);
	Benchmark("Heap Blocks (Run 3)", AllocHeapBlocks, DeallocHeapBlocks);
*/
    return 0;
}

void* AllocHeap(int size) {
	return new CHAR[size];
}

void DeallocHeap(void* ptr) {
	delete [] ptr;
}

void* AllocStaticPool(int size) {
	return allocatorStaticPoolBenchmark.Allocate(size);
}

void DeallocStaticPool(void* ptr) {
	allocatorStaticPoolBenchmark.Deallocate(ptr);
}

void* AllocHeapBlocks(int size) {
	return allocatorHeapBlocksBenchmark.Allocate(size);
}

void DeallocHeapBlocks(void* ptr) {
	allocatorHeapBlocksBenchmark.Deallocate(ptr);
}

void Benchmark(const char* name, AllocFunc allocFunc, DeallocFunc deallocFunc) {
    cout << "Benchmark: " << name << "\n";
    auto totalStart = high_resolution_clock::now();

    // Allocate MAX_BLOCKS blocks MAX_BLOCK_SIZE / 2 sized blocks
    auto start = high_resolution_clock::now();
    for (int i = 0; i < MAX_BLOCKS; i++) {
        memoryPtrs[i] = allocFunc(MAX_BLOCK_SIZE / 2);
    }
    auto end = high_resolution_clock::now();
    cout << "  Allocate MAX_BLOCKS blocks MAX_BLOCK_SIZE / 2 sized blocks: " << duration_cast<microseconds>(end - start).count() << " mcs\n";

    // Deallocate MAX_BLOCKS blocks (every other one)
    start = high_resolution_clock::now();
    for (int i = 0; i < MAX_BLOCKS; i += 2) {
        deallocFunc(memoryPtrs[i]);
    }
    end = high_resolution_clock::now();
    cout << "  Deallocate (every other one): " << duration_cast<microseconds>(end - start).count() << " mcs\n";

    // Allocate MAX_BLOCKS blocks MAX_BLOCK_SIZE sized blocks
    start = high_resolution_clock::now();
    for (int i = 0; i < MAX_BLOCKS; i++) {
        memoryPtrs2[i] = allocFunc(MAX_BLOCK_SIZE);
    }
    end = high_resolution_clock::now();
    cout << "  Allocate MAX_BLOCKS blocks MAX_BLOCK_SIZE sized blocks: " << duration_cast<microseconds>(end - start).count() << " mcs\n";

    // Deallocate MAX_BLOCKS blocks (every other one)
    start = high_resolution_clock::now();
    for (int i = 1; i < MAX_BLOCKS; i += 2) {
        deallocFunc(memoryPtrs[i]);
    }
    end = high_resolution_clock::now();
    cout << "  Deallocate (every other one): " << duration_cast<microseconds>(end - start).count() << " mcs\n";

    // Deallocate MAX_BLOCKS blocks
    start = high_resolution_clock::now();
    for (int i = MAX_BLOCKS - 1; i >= 0; i--) {
        deallocFunc(memoryPtrs2[i]);
    }
    end = high_resolution_clock::now();
    cout << "  Final deallocate: " << duration_cast<microseconds>(end - start).count() << " mcs\n";

    auto totalEnd = high_resolution_clock::now();
    cout << "  TOTAL TIME: " << duration_cast<microseconds>(totalEnd - totalStart).count() << " mcs\n";
}
