/*
 * client_interface.h
 *
 *  Created on: Oct 25, 2014
 *      Author: sorton
 */

#ifndef THREAD_INTERFACE_H_
#define THREAD_INTERFACE_H_

#include <pthread.h>


class thread_interface {
public:
	thread_interface();
	virtual ~thread_interface();

	bool StartThread(void* args);
	bool KillThread(void);

	bool WaitForThread(void** msg);

	pthread_t ThreadId() { return _thread; }

protected:

	void*     _args;

	virtual void* ThreadReactor(void) = 0;

private:

	static void* ThreadEntryFunc(void* _this);

	pthread_t _thread;
};


#endif /* THREAD_INTERFACE_H_ */
