#include <thread>
#include <pthread.h>
#include <iostream>
#include <cstring>

class thread : public std::thread
{
public:
	thread();
	static void setScheduling(std::thread &th, int policy, int priority);
};