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

#include "converters.h"

#include <boost/thread.hpp>

#include "../ConverterCollection.h"
#include "BoolConverter.h"
#include "StringConverter.h"
#include "Uint64Converter.h"
#include "VoidConverter.h"

namespace rsb {
namespace transport {

static bool registered = false;
static boost::mutex registrationMutex;

void registerDefaultConverters() {

	boost::mutex::scoped_lock lock(registrationMutex);
	if (!registered) {
		transport::stringConverterCollection()->registerConverter(
				transport::AbstractConverter<std::string>::Ptr(
						new BoolConverter));
		transport::stringConverterCollection()->registerConverter(
				transport::AbstractConverter<std::string>::Ptr(
						new StringConverter));
		transport::stringConverterCollection()->registerConverter(
				transport::AbstractConverter<std::string>::Ptr(
						new Uint64Converter));
		transport::stringConverterCollection()->registerConverter(
				transport::AbstractConverter<std::string>::Ptr(
						new VoidConverter));
		registered = true;
	}

}

}
}