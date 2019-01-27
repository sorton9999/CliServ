/*
 * ClientParser.h
 *
 *  Created on: Jan 26, 2019
 *      Author: sorton
 */

#ifndef CLIENTPARSER_H_
#define CLIENTPARSER_H_

#include "thread_interface.h"

class ClientParser: public thread_interface {
public:
	ClientParser();
	virtual ~ClientParser();

protected:
	virtual void* ThreadReactor(void);

private:
	bool _loop;
};

#endif /* CLIENTPARSER_H_ */
