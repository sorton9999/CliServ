/*
 * ServiceIF.h
 *
 *  Created on: Jun 9, 2018
 *      Author: sorton
 */

#ifndef SERVICEIF_H_
#define SERVICEIF_H_

#include <stdlib.h>
#include <iostream>
#include <cstdio>


namespace service_if {


class ServiceIF {
public:
	virtual ~ServiceIF() {}
	virtual int SendMsg(std::string msg) = 0;
	virtual int SendMsg(int fd, std::string msg, unsigned int flags) = 0;
	virtual int GetMsg(char** msgRef, unsigned long bufLen) = 0;
	virtual int GetMsg(int fd, char** msgRef, unsigned long bufLen, unsigned int flags) = 0;
};

} /* namespace service_if */

#endif /* SERVICEIF_H_ */
