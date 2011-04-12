/* ============================================================
 *
 * This file is a part of the RSB project.
 *
 * Copyright (C) 2011 by Johannes Wienke <jwienke at techfak dot uni-bielefeld dot de>
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

#include "RSBFactory.h"

#include "introspection/introspection.h"
#include "transport/converter/converters.h"

using namespace std;

namespace rsb {

RSBFactory::RSBFactory() {
	introspection::registerIntrospectionConverters();
	transport::registerDefaultConverters();
}

RSBFactory::~RSBFactory() {
}

SubscriberPtr RSBFactory::createSubscriber(const string &uri) {
	return SubscriberPtr(new Subscriber(uri));
}

patterns::ServerPtr RSBFactory::createServer(const string &uri) {
	return patterns::ServerPtr(new patterns::Server(uri));
}

patterns::RemoteServerPtr RSBFactory::createRemoteServer(const string &uri) {
	return patterns::RemoteServerPtr(new patterns::RemoteServer(uri));
}

}