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

#include <iostream>

#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <rsc/threading/ThreadedTaskExecutor.h>

#include "rsb/transport/Port.h"
#include "rsb/transport/spread/SpreadPort.h"
#include "rsb/filter/AbstractFilter.h"

#include "../../InformerTask.h"

#include "testhelpers.h"

using namespace std;
using namespace rsb;
using namespace rsb::test;
using namespace rsb::filter;
using namespace rsb::transport;
using namespace rsb::spread;
using namespace testing;
using namespace rsc::threading;

TEST(SpreadPortTest, testConstruction)
{
	ASSERT_NO_THROW(PortPtr p = boost::shared_ptr<Port>(new rsb::spread::SpreadPort()));
}

TEST(SpreadPortTest, testConnnection)
{
	PortPtr p = boost::shared_ptr<Port>(new rsb::spread::SpreadPort());
	ASSERT_NO_THROW(p->activate());
}

TEST(SpreadPortTest, testRoundtrip)
{
	// task execution service
	TaskExecutorPtr exec(new ThreadedTaskExecutor);

	// in-process port
	PortPtr p(new SpreadPort());
	ASSERT_NO_THROW(p->activate());

	// filter for joining test group
	AbstractFilterPtr f = AbstractFilterPtr(new ScopeFilter("xcf://blah"));
	f->notifyObserver(p, FilterAction::ADD);

	// domain objects
	// send only one event. Otherwise we need to do a correlation because
	// ordering may change
	const unsigned int numEvents = 1;
	boost::shared_ptr<InformerTask> source(new InformerTask(p, numEvents));
	WaitingObserver observer(numEvents);
	p->setObserver(boost::bind(&WaitingObserver::handler, &observer, _1));

	// activate port and schedule informer
	p->activate();
	exec->schedule(source);

	observer.waitReceived();

	// compare sent and received events
	ASSERT_EQ(source->getEvents().size(), observer.getEvents().size());
	for (unsigned int i = 0; i < source->getEvents().size(); ++i) {
		EventPtr sent = source->getEvents()[i];
		EventPtr received = observer.getEvents()[i];
		EXPECT_EQ(sent->getUUID(), received->getUUID());
		EXPECT_EQ(sent->getType(), received->getType());
		EXPECT_EQ(sent->getURI(), received->getURI());
	}

	// port is deactivated through dtr
	cerr << "SpreadProcessTest finished" << endl;
}
