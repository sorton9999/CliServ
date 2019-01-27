/*
 * client_interface.cpp
 *
 *  Created on: Oct 25, 2014
 *      Author: sorton
 */

#include <signal.h>

#include "client_interface.h"

client_interface::client_interface()
{
	// TODO Auto-generated constructor stub

}

client_interface::~client_interface()
{
	// TODO Auto-generated destructor stub
}

bool client_interface::StartThread(void* args)
{
	_args = args;
	return (pthread_create(&_thread, NULL, ThreadEntryFunc, this) == 0);
}

bool client_interface::KillThread(void)
{
	return (pthread_kill(_thread, 0) == 0);
}

bool client_interface::WaitForThread(void** msg)
{
	return (pthread_join(_thread, msg) == 0);
}

void* client_interface::ThreadEntryFunc(void* _this)
{
	((client_interface*)_this)->ClientReactor(); return NULL;
}
