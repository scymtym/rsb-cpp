/* ============================================================
 *
 * This file is a part of RSB project
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

#include <stdexcept>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "rsb/LocalService.h"

#include "mocks.h"

using namespace std;
using namespace testing;
using namespace rsb;

/**
 * A dummy for testing with the Participant class.
 *
 * @author jwienke
 */
class DummyParticipant: public Participant {
public:
    DummyParticipant(const Scope &scope) :
        Participant(scope, ParticipantConfig()) {
    }
};

TEST(LocalServiceTest, testConstruction)
{

    Scope scope("/this/is/a/test");
    LocalService service(scope);
    EXPECT_EQ(scope, service.getScope());
    EXPECT_TRUE(service.getParticipants().empty());
    EXPECT_TRUE(service.getSubServices().empty());

}

TEST(LocalServiceTest, testParticipants)
{

    Scope scope("/this/is/a/test");
    LocalService service(scope);

    EXPECT_THROW(service.addParticipant(ParticipantPtr(new DummyParticipant(Scope("/this/is/")))), invalid_argument);
    EXPECT_THROW(service.addParticipant(ParticipantPtr(new DummyParticipant(Scope("/not/a/prefix")))), invalid_argument);
    EXPECT_THROW(service.addParticipant(ParticipantPtr(new DummyParticipant(scope))), invalid_argument);

    ParticipantPtr directChild(new DummyParticipant(scope.concat(Scope("/child"))));
    ParticipantPtr indirectChild(new DummyParticipant(scope.concat(Scope("/child/further/"))));

    service.addParticipant(directChild);
    service.addParticipant(indirectChild);

    set<ParticipantPtr> participants = service.getParticipants();
    EXPECT_EQ(size_t(2), participants.size());
    EXPECT_EQ(size_t(1), participants.count(directChild));
    EXPECT_EQ(size_t(1), participants.count(indirectChild));

    service.removeParticipant(indirectChild);
    participants = service.getParticipants();
    EXPECT_EQ(size_t(1), participants.size());
    EXPECT_EQ(size_t(1), participants.count(directChild));

}

TEST(LocalServiceTest, testSubServices)
{

    Scope scope("/this/is/a/test");
    LocalService service(scope);

    NiceMock<MockService> *mockService = new NiceMock<MockService>;
    ServicePtr mockServicePtr(mockService);

    ON_CALL(*mockService, getScope()).WillByDefault(Return(Scope("/this/is")));
    EXPECT_THROW(service.addSubService(mockServicePtr), invalid_argument);
    ON_CALL(*mockService, getScope()).WillByDefault(Return(Scope("/not/a/prefix")));
    EXPECT_THROW(service.addSubService(mockServicePtr), invalid_argument);
    ON_CALL(*mockService, getScope()).WillByDefault(Return(scope));
    EXPECT_THROW(service.addSubService(mockServicePtr), invalid_argument);

    ON_CALL(*mockService, getScope()).WillByDefault(Return(scope.concat(Scope("/direct"))));
    service.addSubService(mockServicePtr);
    EXPECT_EQ(size_t(1), service.getSubServices().size());
    EXPECT_EQ(size_t(1), service.getSubServices().count(mockServicePtr));
    service.removeSubService(mockServicePtr);
    EXPECT_EQ(size_t(0), service.getSubServices().size());

    ON_CALL(*mockService, getScope()).WillByDefault(Return(scope.concat(Scope("/indirect/child"))));
    service.addSubService(mockServicePtr);
    EXPECT_EQ(size_t(1), service.getSubServices().size());
    EXPECT_EQ(size_t(1), service.getSubServices().count(mockServicePtr));
    service.removeSubService(mockServicePtr);
    EXPECT_EQ(size_t(0), service.getSubServices().size());

}