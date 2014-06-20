#include "RTScheduling.h"
#include "Utils.h"

#define SCHEDULING_POLICY SCHED_RR

int initRTCurrentThread(int p_priority)
{
	int ret;
	struct sched_param param;
	
	param.sched_priority = p_priority;
	// pid = 0 -> calling process
	ret = sched_setscheduler(0, SCHEDULING_POLICY, &param);
	if(ret != 0) {
		PRINT_ERR("Failed to set scheduling policy (%d).\n", ret);
		return ret;
	}
	
	return 0;
}

int initRTThreadAttr(pthread_attr_t *p_attributes, unsigned int p_stackSize, int p_priority)
{
	int ret;
	ret = pthread_attr_init(p_attributes);
	if (ret != 0) {
		PRINT_ERR("Failed to init pthread attributes (%d).\n", ret);
		return ret;
	}
	
	ret = pthread_attr_setinheritsched(p_attributes, PTHREAD_EXPLICIT_SCHED);
	if(ret != 0) {
		PRINT_ERR("Failed to set inheritsched of pthread attribute (%d).\n", ret);
		return ret;
	}
	
	ret = pthread_attr_setstacksize(p_attributes, p_stackSize);
	if(ret != 0) {
		PRINT_ERR("Failed to set stacksize of pthread attribute (%d).\n", ret);
		return ret;
	}
	
	ret = pthread_attr_setdetachstate(p_attributes, PTHREAD_CREATE_JOINABLE);
	if(ret != 0) {
		PRINT_ERR("Failed to set detachstate of pthread attribute (%d).\n", ret);
		return ret;
	}
	
	ret = pthread_attr_setscope(p_attributes, PTHREAD_SCOPE_SYSTEM);
	if(ret != 0) {
		PRINT_ERR("Failed to set scope of pthread attribute (%d).\n", ret);
		return ret;
	}
	
	ret = pthread_attr_setschedpolicy(p_attributes, SCHEDULING_POLICY);
	if(ret != 0) {
		PRINT_ERR("Failed to set scheduling policy of pthread attribute (%d).\n", ret);
		return ret;
	}
	
	struct sched_param schedParam;
	// only priority is used by pthreads
	schedParam.sched_priority = p_priority;
	ret = pthread_attr_setschedparam(p_attributes, &schedParam);
	if(ret != 0) {
		PRINT_ERR("Failed to set scheduling priority of pthread attribute (%d).\n", ret);
		return ret;
	}
	
	return 0;
}

int initRTMutexAttr(pthread_mutexattr_t *p_attributes)
{
	int ret;
	ret = pthread_mutexattr_init(p_attributes);
	if(ret != 0) {
		PRINT_ERR("Failed to init mutex attributes (%d).\n", ret);
		return ret;
	}
	
	ret = pthread_mutexattr_setprotocol(p_attributes, PTHREAD_PRIO_INHERIT);
	if(ret != 0) {
		PRINT_ERR("Failed to set mutex protocol (%d).\n", ret);
		return ret;
	}
	
	return 0;

}