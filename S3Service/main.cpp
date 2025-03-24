#include <iostream>
#include <string>
#include <signal.h>

void register_signal(void);
void deregister_signal(void);
void signal_callback(int32_t signum);

auto main(int argc, char* argv[]) -> int
{
    std::cout << "Hello, World!" << std::endl;
 
    return 0;
}

void register_signal(void)
{
	signal(SIGINT, signal_callback);
	signal(SIGILL, signal_callback);
	signal(SIGABRT, signal_callback);
	signal(SIGFPE, signal_callback);
	signal(SIGSEGV, signal_callback);
	signal(SIGTERM, signal_callback);
}

void deregister_signal(void)
{
	signal(SIGINT, nullptr);
	signal(SIGILL, nullptr);
	signal(SIGABRT, nullptr);
	signal(SIGFPE, nullptr);
	signal(SIGSEGV, nullptr);
	signal(SIGTERM, nullptr);
}

void signal_callback(int32_t signum)
{
	deregister_signal();
    // TODO
    // Handle signal
    // Log the signal
    // main loop should be stopped
}