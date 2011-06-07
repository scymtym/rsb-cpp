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

#include "ScopeFilter.h"
#include "FilterObserver.h"

using namespace std;

namespace rsb {
namespace filter {

ScopeFilter::ScopeFilter(const Scope &scope) :
    scope(scope) {
}

ScopeFilter::~ScopeFilter() {
}

string ScopeFilter::getClassName() const {
    return "ScopeFilter";
}

void ScopeFilter::printContents(ostream &stream) const {
    stream << "scope = " << scope;
}

bool ScopeFilter::match(EventPtr e) {
    if ((e->getScope() == this->scope) || e->getScope().isSubScopeOf(
            this->scope)) {
        return true;
    } else {
        return false;
    }
}

void ScopeFilter::notifyObserver(FilterObserverPtr fo, FilterAction::Types at) {
    fo->notify(this, at);
}

Scope ScopeFilter::getScope() {
    return scope;
}

}
}
