#include "Assembly.h"

#include <boost/date_time/microsec_time_clock.hpp>

using namespace std;

using namespace boost;
using namespace boost::posix_time;

using namespace rsc::logging;
using namespace rsc::threading;

using namespace rsb::protocol;

namespace rsb {
namespace spread {

Assembly::Assembly(rsb::protocol::NotificationPtr n) :
    logger(Logger::getLogger("rsb.spread.Assembly[" + n->id() + "]")), receivedParts(0),
    birthTime(microsec_clock::local_time()) {
    store.resize(n->num_data_parts() + 1);
    add(n);
}

Assembly::~Assembly() {
}

string Assembly::getData(const unsigned int &i) const {
    return this->store[i]->data().binary();
}

string *Assembly::getCompleteData() const {
    RSCTRACE(logger, "Joining data parts");
    assert(isComplete());
    // Concatenate data parts
    string* result = new string(getData(0));
    for (unsigned int i = 1; i < this->store.size(); ++i) {
        result->append(getData(i));
    }
    return result;
}

unsigned int Assembly::add(NotificationPtr n) {
    RSCTRACE(logger, "Adding notification " << n->id()
             << " (part " << n->data_part() << "/" << this->store.size()
             << ") to assembly");

    store[n->data_part()] = n;
    return receivedParts++;
}

bool Assembly::isComplete() const {
    return this->receivedParts == this->store.size();
}

unsigned int Assembly::age() const {
    return (microsec_clock::local_time() - this->birthTime).total_seconds();
}

AssemblyPool::PruningTask::PruningTask(Pool &pool, boost::recursive_mutex &poolMutex) :
    PeriodicTask(4000),
    logger(Logger::getLogger("rsb.spread.AssemblyPool.PruningTask")),
    pool(pool), poolMutex(poolMutex) {
}

void AssemblyPool::PruningTask::execute() {
    boost::recursive_mutex::scoped_lock lock(this->poolMutex);

    RSCDEBUG(logger, "Scanning for old assemblies");
    for (Pool::iterator it = this->pool.begin(); it != this->pool.end(); ++it) {
        if (it->second->age() > 20) {
            RSCDEBUG(logger, "Pruning old assembly " << it->second);
            this->pool.erase(it);
        }
    }
}

AssemblyPool::AssemblyPool() :
    logger(Logger::getLogger("rsb.spread.AssemblyPool")),
    pruningTask(new PruningTask(this->pool, this->poolMutex)) {
    this->executor.schedule(this->pruningTask);
}

AssemblyPool::~AssemblyPool() {
    this->pruningTask->cancel();
    this->pruningTask->waitDone();
}

shared_ptr<string> AssemblyPool::add(NotificationPtr notification) {
    boost::recursive_mutex::scoped_lock lock(this->poolMutex);

    Pool::iterator it = this->pool.find(notification->id());
    string *result = 0;
    if (it != this->pool.end()) {
        // Push message to existing Assembly
        AssemblyPtr assembly = it->second;
        RSCTRACE(logger, "Adding notification " << notification->id() << " to existing assembly "
                 << assembly);
        assembly->add(notification);
        if (assembly->isComplete()) {
            result = assembly->getCompleteData();
            this->pool.erase(it);
        }
    } else {
        // Create new Assembly
        RSCTRACE(logger, "Creating new assembly for notification " << notification->id());
        this->pool.insert(make_pair(notification->id(), AssemblyPtr(new Assembly(notification))));
    }
    RSCTRACE(logger, "dataPool size: " << this->pool.size());

    return shared_ptr<string>(result);
}


}
}