/*
 * client_interface.cpp
 *
 *  Created on: Oct 25, 2014
 *      Author: sorton
 */

#include <signal.h>

#include "thread_interface.h"

thread_interface::thread_interface()
{
	_thread = 0;
	_args = NULL;
}

thread_interface::~thread_interface()
{
}

bool thread_interface::StartThread(void* args)
{
	_args = args;
	return (pthread_create(&_thread, NULL, ThreadEntryFunc, this) == 0);
}

bool thread_interface::KillThread(void)
{
	return (pthread_kill(_thread, 0) == 0);
}

bool thread_interface::WaitForThread(void** msg)
{
	return (pthread_join(_thread, msg) == 0);
}

void* thread_interface::ThreadEntryFunc(void* _this)
{
	((thread_interface*)_this)->ThreadReactor(); return NULL;
}
