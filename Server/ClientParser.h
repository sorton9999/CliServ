/*
 * ClientParser.h
 *
 *  Created on: Jan 26, 2019
 *      Author: sorton
 */

#ifndef CLIENTPARSER_H_
#define CLIENTPARSER_H_

#include <mutex>
#include <map>
#include "thread_interface.h"

struct ClientData
{
	pthread_t   ClientHandle;
	int         ClientFd;
	std::string ClientName;

	friend bool operator==(const ClientData& lhs, const ClientData& rhs)
	{
		return ((lhs.ClientFd == rhs.ClientFd) &&
				(lhs.ClientHandle == rhs.ClientHandle) &&
				(lhs.ClientName == rhs.ClientName));
	}
	friend bool operator!=(const ClientData& lhs, const ClientData& rhs)
	{
		return !(lhs == rhs);
	}
};

class ClientParser: public thread_interface {
public:
	typedef std::map<pthread_t, ClientData> ClientMap;
	typedef std::map<pthread_t, ClientData>::iterator ClientIter;

	ClientParser();
	virtual ~ClientParser();

	void       CleanUp(void);
	ClientIter FindClientByName(std::string name);
	ClientIter FindClientByHandle(pthread_t handle);

protected:
	virtual void* ThreadReactor(void);

private:
	static ClientMap  client_store;
	ClientIter client_ref;
	std::mutex Mutex;
};

#endif /* CLIENTPARSER_H_ */
