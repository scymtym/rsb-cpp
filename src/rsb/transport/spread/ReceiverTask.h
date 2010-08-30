/* ============================================================
 *
 * This file is a part of the RSB project
 *
 * Copyright (C) 2010 by Sebastian Wrede <swrede at techfak dot uni-bielefeld dot de>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef RECEIVERTASK_H_
#define RECEIVERTASK_H_

#include "../../RSBEvent.h"
#include "SpreadConnection.h"
#include "../QueueAndDispatchTask.h"
#include "../Port.h"

#include <log4cxx/logger.h>

namespace rsb {

namespace spread {

typedef boost::shared_ptr<rsb::transport::QueueAndDispatchTask< rsb::RSBEventPtr > > QADPtr;

class ReceiverTask {
public:
	ReceiverTask(SpreadConnectionPtr s, rsb::transport::ConverterMapPtr c, QADPtr q);
	virtual ~ReceiverTask();

	void execute(rsb::util::Task<void>* t);

	void cancel() {
		cancelRequested = true;
	}

private:
	log4cxx::LoggerPtr logger;
	volatile bool cancelRequested;
	SpreadConnectionPtr con;
	rsb::transport::ConverterMapPtr converters;
	QADPtr qad;
};

}

}

#endif /* RECEIVERTASK_H_ */