/* ============================================================
 *
 * This file is part of the RSB project
 *
 * Copyright (C) 2012, 2014 Jan Moringen <jmoringe@techfak.uni-bielefeld.de>
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

#include <iostream>
#include <fstream>

#include <rsc/misc/SignalWaiter.h>

#include <rsb/Factory.h>
#include <rsb/Handler.h>

#include <rsb/converter/Repository.h>
#include <rsb/converter/RosettaConverter.h>

// See ../CMakeLists.txt for the generation of this file.
// The generated file can be found in ${BUILD_DIR}/protobuf_converter
#include <rosetta_converter/SimpleImage_Bottle.hh>

using namespace std;

using namespace boost;

using namespace rsb;
using namespace rsb::converter;

// The generated data-holder class is in this namespace.
using namespace tutorial::rosetta_converter;

void printImage(boost::shared_ptr<SimpleImage> image) {
    cout << "received " << image->width() << "x" << image->height() << " image"
            << endl;
}

int main() {

    rsc::misc::initSignalWaiter();

    // Register a specific template instanciation of the
    // RosettaConverter for our SimpleImage data type.
    boost::shared_ptr< RosettaConverter<rosetta::MechanismBottle, SimpleImage> >
        converter(new  RosettaConverter<rosetta::MechanismBottle, SimpleImage>());
    converterRepository<string>()->registerConverter(converter);

    ifstream stream("/tmp/SimpleImage.bottle", ios::binary);
    char buffer[24];
    stream.read(buffer, 24);
    string data(buffer, buffer + 24);
    RosettaConverter<rosetta::MechanismBottle, SimpleImage> c;
    printImage(static_pointer_cast<SimpleImage>(c.deserialize("rosetta::MechanismBottle:tutorial::rosetta_converter::SimpleImage",
                                                              data).second));

    // Create a listener to receive SimpleImage data.
    ListenerPtr listener = getFactory().createListener(Scope("/tutorial/converter"));
    listener->addHandler(HandlerPtr(new DataFunctionHandler<SimpleImage> (&printImage)));

    return rsc::misc::suggestedExitCode(rsc::misc::waitForSignal());

}
