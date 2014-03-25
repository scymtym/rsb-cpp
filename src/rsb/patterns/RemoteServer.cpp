/* ============================================================
 *
 * This file is a part of RSB project
 *
 * Copyright (C) 2010 by Johannes Wienke <jwienke at techfak dot uni-bielefeld dot de>
 *               2011 Jan Moringen <jmoringe@techfak.uni-bielefeld.de>
 *
 * This file may be licensed under the terms of the
 * GNU Lesser General Public License Version 3 (the ``LGPL''),
 * or (at your option) any later version.
 *
 * Software distributed under the License is distributed
 * on an ``AS IS'' basis, WITHOUT WARRANTY OF ANY KIND, either
 * express or implied. See the LGPL for the specific language
 * governing rights and limitations.
 *
 * You should have received a copy of the LGPL along with this
 * program. If not, go to http://www.gnu.org/licenses/lgpl.html
 * or write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * The development of this software was supported by:
 *   CoR-Lab, Research Institute for Cognition and Robotics
 *     Bielefeld University
 *
 * ============================================================ */

#include "RemoteServer.h"

#include <stdexcept>

#include <boost/format.hpp>
#include <boost/thread/mutex.hpp>

#include <rsc/runtime/TypeStringTools.h>
#include <rsc/misc/UUID.h>

#include "../Factory.h"
#include "../Handler.h"
#include "../MetaData.h"
#include "../EventId.h"

using namespace std;

using namespace boost;

using namespace rsc::runtime;
using namespace rsc::logging;
using namespace rsc::threading;

namespace rsb {
namespace patterns {

class WaitingEventHandler: public Handler {
public:
    typedef boost::recursive_mutex MutexType;
private:
    LoggerPtr logger;

    MutexType mutex;

    map<EventId, RemoteServer::FuturePtr> inprogress;
public:

    WaitingEventHandler(LoggerPtr logger) :
        logger(logger) {
    }

    string getClassName() const {
        return "WaitingEventHandler";
    }

    MutexType& getMutex() {
        return this->mutex;
    }

    void handle(EventPtr event) {
        if (!event
            || event->getCauses().empty()
            || (event->getMethod() != "REPLY")) {
            RSCTRACE(logger, "Received uninteresting event " << event);
            return;
        }
        EventId requestId = *event->getCauses().begin();
        {
            MutexType::scoped_lock lock(mutex);

            if (!this->inprogress.count(requestId)) {
                RSCTRACE(logger, "Received uninteresting event " << event);
                return;
            }

            RSCDEBUG(logger, "Received reply event " << event);

            RemoteServer::FuturePtr result = this->inprogress[requestId];
            if (event->mutableMetaData().hasUserInfo("rsb:error?")) {
                assert(event->getType() == typeName<string>());
                result->setError(str(format("Error calling remote method '%1%': %2%")
                                     % "TODO: obtain method name"
                                     % *(boost::static_pointer_cast<string>(event->getData()))));
            } else {
                result->set(event);
            }
            this->inprogress.erase(requestId);
        }
    }

    void addCall(const EventId& requestId, RemoteServer::FuturePtr result) {
        MutexType::scoped_lock lock(this->mutex);
        this->inprogress.insert(make_pair(requestId, result));
    }

};

RemoteServer::RemoteServer(const Scope&            scope,
                           const ParticipantConfig &listenerConfig,
                           const ParticipantConfig &informerConfig)
    : Participant(scope, listenerConfig), // TODO do this properly
      logger(Logger::getLogger(str(format("rsb.patterns.RemoteServer[%1%]")
                                   % scope.toString()))),
      listenerConfig(listenerConfig), informerConfig(informerConfig) {
    // TODO check that this server is alive...
    // TODO probably it would be a good idea to request some method infos from
    //      the server, e.g. for type checking
}

RemoteServer::~RemoteServer() {
}

RemoteServer::MethodSet RemoteServer::getMethodSet(const string& methodName) {

    boost::mutex::scoped_lock lock(methodSetMutex);

    if (!methodSets.count(methodName)) {

        const Scope methodScope = getScope()->concat(Scope("/" + methodName));

        // start a listener to wait for the reply

        ListenerPtr listener
            = getFactory().createListener(methodScope,
                                          this->listenerConfig);

        boost::shared_ptr<WaitingEventHandler> handler(
                new WaitingEventHandler(logger));
        listener->addHandler(handler);

        // informer for requests
        InformerBasePtr informer
            = getFactory().createInformerBase(methodScope,
                                              "",
                                              this->informerConfig);

        MethodSet set;
        set.methodName = methodName;
        set.handler = handler;
        set.replyListener = listener;
        set.requestInformer = informer;

        methodSets[methodName] = set;

    }

    return methodSets[methodName];

}

RemoteServer::FuturePtr RemoteServer::callAsync(const string& methodName, EventPtr data) {

    RSCDEBUG(logger, "Calling method " << methodName << " with data " << data);

    // TODO check that the desired method exists
    MethodSet methodSet = getMethodSet(methodName);
    FuturePtr result;
    {
        WaitingEventHandler::MutexType::scoped_lock
            lock(methodSet.handler->getMutex());

        data->setScopePtr(methodSet.requestInformer->getScope());
        data->setMethod("REQUEST");
        methodSet.requestInformer->publish(data);
        result.reset(new Future<EventPtr>());
        methodSet.handler->addCall(data->getEventId(), result);
    }

    return result;
}

EventPtr RemoteServer::call(const string& methodName,
                            EventPtr      data,
                            unsigned int  maxReplyWaitTime) {
    return callAsync(methodName, data)->get(maxReplyWaitTime);
}

}
}
