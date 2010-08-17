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

#include "transport/Router.h"
#include "util/Task.h"
#include "util/PeriodicTask.h"
#include "transport/Port.h"
#include "transport/spread/SpreadPort.h"
#include "InformerTask.h"
#include "Subscription.h"
#include "filter/AbstractFilter.h"
#include "filter/ScopeFilter.h"

#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <log4cxx/basicconfigurator.h>
#include <iostream>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

using namespace std;
using namespace rsb;
using namespace rsb::test;
using namespace rsb::filter;
using namespace rsb::util;
using namespace rsb::transport;
using namespace rsb::spread;
using namespace testing;

TEST(RSBTest, testRoundtrip)
{
	// task execution service
	TaskExecutorVoidPtr exec(new TaskExecutor<void> ());

	// router instantiation
	RouterPtr r(new Router(TransportFactory::SPREAD, TransportFactory::SPREAD));
	r->activate();

	// create subscription
	SubscriptionPtr s(new Subscription());
	AbstractFilterPtr f(new ScopeFilter("xcf://blah"));
	s->appendFilter(f);

	// domain objects
	boost::shared_ptr<InformerTask> source(new InformerTask(r->getOutPort()));
	s->appendAction(boost::bind(&InformerTask::handler, source.get(), _1));

	// add subscription to router
	r->subscribe(s);

	// activate port and schedule informer
	TaskPtr task_source = exec->schedulePeriodic<InformerTask> (source, 0);

	// wait *here* for shutdown as this is not known to the Port
	{
		boost::recursive_mutex::scoped_lock lock(source->m);
		while (source->c != 10) {
			source->cond.wait(lock);
		}
	}

	// stop and join all threads
	exec->join(task_source); // no cancel needed as done already locally, see execute()
	// port is deactivated through dtr
	cerr << "RSBTest finished" << endl;
}

int main(int argc, char* argv[]) {
	log4cxx::BasicConfigurator::configure();
    InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();

}
