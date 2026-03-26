#include "core/include/runtimeGuard.hpp"

int main() {
    runtime_guard::initialize();
    
    // Deliberate native memory corruption
    volatile int* ptr = nullptr;
    *ptr = 42;
    
    return 0;
}
