/*
 * ServiceIF.h
 *
 *  Created on: Jun 9, 2018
 *      Author: sorton
 */

#ifndef SERVICEIF_H_
#define SERVICEIF_H_

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <cstdio>


namespace service_if {


class ServiceIF {
public:
	virtual ~ServiceIF() {}
	virtual int SendMsg(std::string msg) = 0;
	virtual std::string GetMsg(void) = 0;
};

} /* namespace service_if */

#endif /* SERVICEIF_H_ */
