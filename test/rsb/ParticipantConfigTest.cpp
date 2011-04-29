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

#include "rsb/ParticipantConfig.h"

using namespace std;
using namespace testing;
using namespace rsb;

TEST(TransportTest, testCreation)
{

    EXPECT_THROW(ParticipantConfig::Transport(""), invalid_argument);
    EXPECT_EQ("a test", ParticipantConfig::Transport("a test").getName());

}

TEST(ParticipantConfigTest, testCreation)
{

    ParticipantConfig config;
    EXPECT_TRUE(config.getTransports().empty());
    EXPECT_EQ(QualityOfServiceSpec(), config.getQualityOfServiceSpec());

}

TEST(ParticipantConfigTest, testQualityOfServiceSpecManipulation)
{

    ParticipantConfig config;

    QualityOfServiceSpec spec(QualityOfServiceSpec::ORDERED,
            QualityOfServiceSpec::UNRELIABLE);
    EXPECT_NE(spec, config.getQualityOfServiceSpec());
    config.setQualityOfServiceSpec(spec);
    EXPECT_EQ(spec, config.getQualityOfServiceSpec());

}

TEST(ParticipantConfigTest, testTransportManipulation)
{

    ParticipantConfig config;

    ParticipantConfig::Transport t1("t1");
    ParticipantConfig::Transport t2("t2");

    EXPECT_EQ(size_t(0), config.getTransports().size());
    config.addTransport(t1);
    EXPECT_EQ(size_t(1), config.getTransports().size());
    config.addTransport(t1);
    EXPECT_EQ(size_t(1), config.getTransports().size());
    config.addTransport(t2);
    EXPECT_EQ(size_t(2), config.getTransports().size());

    config.removeTransport(ParticipantConfig::Transport("unknown"));
    EXPECT_EQ(size_t(2), config.getTransports().size());
    config.removeTransport(ParticipantConfig::Transport(t1.getName()));
    EXPECT_EQ(size_t(1), config.getTransports().size());
    config.removeTransport(ParticipantConfig::Transport(t2));
    EXPECT_EQ(size_t(0), config.getTransports().size());

    set<ParticipantConfig::Transport> newSet;
    newSet.insert(ParticipantConfig::Transport("foo"));
    config.setTransports(newSet);
    EXPECT_EQ(newSet, config.getTransports());

}