/* ============================================================
 *
 * This file is part of the RSB project
 *
 * Copyright (C) 2011 Jan Moringen <jmoringe@techfak.uni-bielefeld.de>
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

#pragma once

#include <algorithm>
#include <iterator>

#include <boost/shared_ptr.hpp>

#include "ConverterSelectionStrategy.h"

namespace rsb {
namespace converter {

/**
 * Implementations of this interface can be used to perfrom @ref
 * Converter selection based on predicate evaluation. Depending on the
 * context, the @ref match method of predicates is invoked with either
 * a wire-schema string or a data-type string in order to decide
 * whether an associated converter should be selected.
 *
 * @author jmoringe
 */
class ConverterPredicate: public rsc::runtime::Printable {
public:
    /**
     * Return @c true if the @ref Converter associated to this
     * predicate should be selected for @a key.
     *
     * @param key Depending on the context, either a wire-schema or a
     *            data-type for which a converter has to be selected.
     * @return @c true if the associated converter should be selected;
     * @c false otherwise.
     */
    virtual bool match(const std::string &key) const = 0;
};

typedef boost::shared_ptr<ConverterPredicate> ConverterPredicatePtr;

/**
 * A predicate that unconditionally causes its associated @ref
 * Converter to be selected.
 *
 * @author jmoringe
 */
class AlwaysApplicable: public ConverterPredicate {
public:
    bool match(const std::string &/*key*/) const {
        return true;
    }

    std::string getClassName() const {
        return "AlwaysApplicable";
    }

    void printContents(std::ostream &/*stream*/) const {
    }
};

/**
 * Objects of this class are used to perform @ref Converter selection
 * via a chain-of-responsibility strategy. A list of predicates and
 * associated converters is maintained. Converter selection queries
 * are processed by traversing the list and selected the first
 * converter the associated predicate of which matches the query
 * wire-schema or data-type.
 *
 * @author jmoringe
 */
template <typename WireType>
class PredicateConverterList: public ConverterSelectionStrategy<WireType> {
public:
    typedef typename ConverterSelectionStrategy<WireType>::ConverterPtr ConverterPtr;
    typedef std::list< std::pair<ConverterPredicatePtr, ConverterPtr> > ConverterList;

    /**
     * Construct a new PredicateConverterList object, initializing its
     * @ref Converter to the elements of the iterator range given by
     * @a start and @a end.
     *
     * @param begin A ForwardIterator the value type of which is a
     *              std::pair of a @ref ConverterPredicatePtr and a @ref
     *              Converter<WireType>::Ptr.
     * @param end An associated end iterator.
     */
    template <typename ForwardIterator>
    PredicateConverterList(ForwardIterator begin, ForwardIterator end) {
        std::copy(begin, end, std::inserter(this->converters,
                                            this->converters.begin()));
    }

    ConverterPtr getConverter(const std::string &key) const {
        for (typename ConverterList::const_iterator it = this->converters.begin();
             it != this->converters.end(); ++it) {
            if (it->first->match(key)) {
                return it->second;
            }
        }
        return ConverterPtr();
    }

    std::string getClassName() const {
        return "PredicateConverterList";
    }

    void printContents(std::ostream &stream) const {
        stream << "converters = " << this->converters;
    }
private:
    ConverterList converters;
};

}
}
