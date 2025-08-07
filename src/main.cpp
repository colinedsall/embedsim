#include <iostream>
#include <signal.h>
#include <execinfo.h>
#include <unistd.h>
#include "clock.hpp"
#include "io.hpp"
#include "system.hpp"

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

int main() {
    // Set up signal handler for segmentation faults
    signal(SIGSEGV, segfault_handler);
    
    std::cout << "DEBUG: Starting main()" << std::endl;
    
    System system;
    std::cout << "DEBUG: System object created" << std::endl;
    
    system.run();
    std::cout << "DEBUG: System run completed" << std::endl;

    return 0;
}