/* ============================================================
 *
 * This file is a part of RSB project
 *
 * Copyright (C) 2010 by Johannes Wienke <jwienke at techfak dot uni-bielefeld dot de>
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

#include <stdexcept>

#include "QualityOfServiceSpec.h"
#include "rsb/rsbexports.h"
#include "Exception.h"

namespace rsb {

/**
 * An exception indicating that a desired QualityOfServiceSpec cannot be reached
 * by a transport.
 *
 * @author jwienke
 */
class RSB_EXPORT UnsupportedQualityOfServiceException: Exception {
public:

	UnsupportedQualityOfServiceException(const std::string& message,
			const QualityOfServiceSpec& spec);
	virtual ~UnsupportedQualityOfServiceException() throw ();

	virtual const char* what() const throw ();

private:

	std::string message;
	QualityOfServiceSpec spec;

};

}

