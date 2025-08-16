#include <iostream>
#include <signal.h>
#include <execinfo.h>
#include <unistd.h>
#include "clock.hpp"
#include "system.hpp"

// Global system pointer for cleanup
System* g_system = nullptr;

void segfault_handler(int sig) {
    void *array[10];
    size_t size;

    // Get void*'s for all entries on the stack
    size = backtrace(array, 10);

    // Print out all the frames to stderr
    fprintf(stderr, "Error: signal %d:\n", sig);
    backtrace_symbols_fd(array, size, STDERR_FILENO);
    exit(1);
}

void cleanup_handler(int sig) {
    std::cout << "\nReceived signal " << sig << ", cleaning up..." << std::endl;
    
    if (g_system) {
        // Clean up system resources
        g_system->~System();
    }
    
    std::cout << "Cleanup complete. Exiting." << std::endl;
    exit(0);
}

int main() {
    // Set up signal handlers for segmentation faults and cleanup
    signal(SIGSEGV, segfault_handler);
    signal(SIGINT, cleanup_handler);   // Ctrl+C
    signal(SIGTERM, cleanup_handler);  // Termination signal
    
    std::cout << "DEBUG: Starting main()" << std::endl;
    
    System system;
    g_system = &system;  // Store for cleanup
    std::cout << "DEBUG: System object created" << std::endl;
    
    system.run();
    std::cout << "DEBUG: System run completed" << std::endl;

    return 0;
}