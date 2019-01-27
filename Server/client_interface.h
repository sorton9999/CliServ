/*
 * client_interface.h
 *
 *  Created on: Oct 25, 2014
 *      Author: sorton
 */

#ifndef CLIENT_INTERFACE_H_
#define CLIENT_INTERFACE_H_

#include <pthread.h>


class client_interface {
public:
	client_interface();
	virtual ~client_interface();

	bool StartThread(void* args);
	bool KillThread(void);

	bool WaitForThread(void** msg);

	pthread_t ThreadId() { return _thread; }

protected:

	void*     _args;

	virtual void* ClientReactor(void) = 0;

private:

	static void* ThreadEntryFunc(void* _this);

	pthread_t _thread;
};


#endif /* CLIENT_INTERFACE_H_ */
