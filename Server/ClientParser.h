/*
 * ClientParser.h
 *
 *  Created on: Jan 26, 2019
 *      Author: sorton
 */

#ifndef CLIENTPARSER_H_
#define CLIENTPARSER_H_

#include "client_interface.h"

class ClientParser: public client_interface {
public:
	ClientParser();
	virtual ~ClientParser();

protected:
	virtual void* ClientReactor(void);

private:
	bool _loop;
};

#endif /* CLIENTPARSER_H_ */
