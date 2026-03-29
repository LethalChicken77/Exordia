#ifndef CLOSE_HANDLER
#define CLOSE_HANDLER

#include <csignal>
#include <iostream>
#include "utils/console.hpp"
// Only to be used by main

void OnExit()
{
    Console::saveLog();
    std::cout << "Exited successfully" << std::endl;
}

void OnSignal(int sig)
{
    #ifdef _WIN32
    switch(sig)
    {
        case SIGSEGV:
            std::cout << "Segmentation fault (core dumped)" << std::endl; // Because windows doesn't like to tell me things
            break;
        case SIGFPE:
            std::cout << "Floating point error (core dumped)" << std::endl; // Because windows doesn't like to tell me things
            break;
        case SIGILL:
            std::cout << "Illegal instruction (core dumped)" << std::endl; // Does this one dump the core? Who knows!
            break;
        case SIGABRT:
            std::cout << "Aborted" << std::endl;
            break;
        case SIGTERM:
            std::cout << "Exited successfully" << std::endl;
            break;
    }
    #endif
    // Save log file
    Console::saveLog();
}

void HandleSignals()
{
    atexit(OnExit);

    std::signal(SIGABRT, OnSignal);
    std::signal(SIGINT, OnSignal);
    std::signal(SIGTERM, OnSignal);
    std::signal(SIGSEGV, OnSignal);
    std::signal(SIGFPE, OnSignal);

}
#endif