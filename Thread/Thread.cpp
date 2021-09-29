#include "Thread.h"

thread::thread() {}
void thread::setScheduling(std::thread &th, int policy, int priority) {
	sched_param sch_params;
	sch_params.sched_priority = priority;
	if(pthread_setschedparam(th.native_handle(), policy, &sch_params)) {
		std::cerr << "Failed to set Thread scheduling : " << std::strerror(errno) << std::endl;
	}
}