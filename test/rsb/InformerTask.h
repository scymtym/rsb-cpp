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

#ifndef INFORMERTASK_H_
#define INFORMERTASK_H_

#include <boost/thread/recursive_mutex.hpp>
#include <boost/thread/condition.hpp>

#include <rsc/threading/RepetitiveTask.h>

#include "rsb/transport/Port.h"

namespace rsb {
namespace test {

/**
 * @author swrede
 */
class InformerTask: public rsc::threading::RepetitiveTask {
public:
	InformerTask(rsb::transport::PortPtr p, const unsigned int &numEvents);
	virtual ~InformerTask();

	void execute();

	std::vector<RSBEventPtr> getEvents();

private:
	unsigned int numEvents;
	unsigned int sentEvents;
	rsb::transport::PortPtr port;
	std::vector<RSBEventPtr> events;

};

/**
 * @author jwienke
 */
class WaitingObserver {
public:

	WaitingObserver(const unsigned int &desiredEvents);

	void handler(RSBEventPtr e);

	void waitReceived();

	std::vector<RSBEventPtr> getEvents();

private:

	unsigned int desiredEvents;
	unsigned int receivedEvents;
	boost::recursive_mutex m;
	boost::condition condition;
	std::vector<RSBEventPtr> events;

};

}
}

#endif /* INFORMERTASK_H_ */
