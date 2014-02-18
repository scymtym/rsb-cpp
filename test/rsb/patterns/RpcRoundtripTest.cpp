/* ============================================================
 *
 * This file is a part of RSB project
 *
 * Copyright (C) 2014 by Johannes Wienke <jwienke at techfak dot uni-bielefeld dot de>
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

#include <utility>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "rsb/EventId.h"
#include "rsb/ParticipantConfig.h"
#include "rsb/patterns/Server.h"
#include "rsb/patterns/RemoteServer.h"

using namespace std;
using namespace testing;
using namespace rsb;
using namespace rsb::patterns;

class TestCallback: public Server::Callback<boost::uint64_t, string> {
public:

    boost::shared_ptr<string> call(const string& methodName,
            boost::shared_ptr<boost::uint64_t> input) {

        if (*input != 23) {
            return boost::shared_ptr<string>(new string("wrong input"));
        }

        if (methodName != "myMethod") {
            return boost::shared_ptr<string>(new string("wrong method"));
        }

        return boost::shared_ptr<string>(new string("OK"));
    }

};

TEST(RpcRoundtripTest, testRoundtrip) {

    ParticipantConfig config;
    config.addTransport(ParticipantConfig::Transport("inprocess"));

    const Scope scope("/rpc/test/scope");
    const string methodName = "myMethod";

    Server server(scope, config, config);
    server.registerMethod(methodName, Server::CallbackPtr(new TestCallback));

    RemoteServer remote(scope, config, config);

    const boost::uint64_t value = 23;
    boost::shared_ptr<string> reply =
            remote.callAsync<string, boost::uint64_t>(methodName,
                    boost::shared_ptr<boost::uint64_t>(
                            new boost::uint64_t(value))).get(
                    50000);

    EXPECT_EQ("OK", *reply);

}
