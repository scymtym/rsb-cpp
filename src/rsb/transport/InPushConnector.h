/* ============================================================
 *
 * This file is part of the RSB project
 *
 * Copyright (C) 2011, 2012 Jan Moringen <jmoringe@techfak.uni-bielefeld.de>
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

#pragma once

#include <list>

#include <boost/shared_ptr.hpp>

#include "../eventprocessing/Handler.h"
#include "../ParticipantConfig.h"
#include "InConnector.h"
#include "rsb/rsbexports.h"

namespace rsb {
namespace transport {

/**
 * Objects of classes which implement this specialized @ref
 * InConnector interface provide the ability to receive events in
 * push-style manner by means of one transport mechanism.
 *
 * @author jmoringe
 */
class RSB_EXPORT InPushConnector: public virtual InConnector {
public:
    typedef boost::shared_ptr<InPushConnector> Ptr;

    virtual ~InPushConnector();

    virtual void addHandler(eventprocessing::HandlerPtr handler);
    virtual void removeHandler(eventprocessing::HandlerPtr handler);

    /**
     * A temporary hack to pass an ErrorStrategy to in push connectors so that
     * they can be instructed by clients on what to do in case of asynchronous
     * receive errors. The current convention is that @c ERROR_STRATEGY_LOG and
     * @c ERROR_STRATEGY_PRINT shall use the declared channel for displaying the
     * error once and afterwards connectors should stop processing.
     *
     * @param strategy the new error strategy to use
     */
    virtual void setErrorStrategy(ParticipantConfig::ErrorStrategy strategy);
protected:
    typedef std::list<eventprocessing::HandlerPtr> HandlerList;

    HandlerList handlers;
};

typedef InPushConnector::Ptr InPushConnectorPtr;

}
}
