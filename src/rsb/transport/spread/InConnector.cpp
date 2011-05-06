/* ============================================================
 *
 * This file is part of the RSB project
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

#include "InConnector.h"

#include <rsc/threading/ThreadedTaskExecutor.h>

using namespace std;

using namespace rsb::eventprocessing;
using namespace rsc::logging;
using namespace rsc::runtime;
using namespace rsc::threading;

namespace rsb {
namespace spread {

rsb::transport::InConnector *InConnector::create(const Properties& args) {
    static LoggerPtr logger = Logger::getLogger("rsb.spread.InConnector");
    RSCDEBUG(logger, "creating InConnector with properties " << args);

    return new InConnector(args.get<string> ("host", defaultHost()),
                           args.get<unsigned int> ("port", defaultPort()),
                           args.get<ConverterNames>("converters", ConverterNames()));
}

InConnector::InConnector(const string &host, unsigned int port,
                         const ConverterNames &converters) :
    transport::ConverterSelectingInConnector<string>(converters),
    logger(Logger::getLogger("rsb.spread.InConnector")), active(false),
            connector(new SpreadConnector(host, port)) {
    this->exec = TaskExecutorPtr(new ThreadedTaskExecutor);
    // TODO check if it makes sense and is possible to provide a weak_ptr to the ctr of StatusTask
    //st = boost::shared_ptr<StatusTask>(new StatusTask(this));
    this->rec = boost::shared_ptr<ReceiverTask>(new ReceiverTask(
                                                    this->connector->getConnection(), HandlerPtr(),
                                                    this));
}

InConnector::~InConnector() {
    if (this->active) {
        deactivate();
    }
}

string InConnector::getClassName() const {
    return "InConnector";
}

void InConnector::printContents(ostream &stream) const {
    stream << "connector = " << connector;
}

void InConnector::activate() {
    this->connector->activate();

    // (re-)start threads
    this->exec->schedule(rec);
    //this->exec->schedule(st);
    this->active = true;
}

void InConnector::deactivate() {
    this->rec->cancel();
    // killing spread connection, exception thrown to rec thread which
    // should be handled specifically as the cancel flag was set
    this->connector->deactivate();
    this->rec->waitDone();
    this->active = false;
}

void InConnector::setQualityOfServiceSpecs(const QualityOfServiceSpec &specs) {
    this->connector->setQualityOfServiceSpecs(specs);
    if (specs.getReliability() >= QualityOfServiceSpec::RELIABLE) {
        this->rec->setPruning(false);
    } else {
        this->rec->setPruning(true);
    }
}

void InConnector::addHandler(HandlerPtr handler) {
    assert(this->handlers.empty());
    transport::InConnector::addHandler(handler);
    this->rec->setHandler(this->handlers.front());
}

void InConnector::removeHandler(HandlerPtr handler) {
    transport::InConnector::addHandler(handler);
    this->rec->setHandler(HandlerPtr());
}

void InConnector::setScope(const Scope &scope) {
    connector->join(connector->makeGroupName(scope));
}

}
}
