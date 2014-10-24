/* ============================================================
 *
 * This file is part of the RSB project.
 *
 * Copyright (C) 2014 Jan Moringen <jmoringe@techfak.uni-bielefeld.de>
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
 * ============================================================  */

#include <rsc/logging/Logger.h>
#include <rsc/plugins/Provider.h>

#include <../ParticipantConfig.h>
#include <../Factory.h>

#include <../converter/Repository.h>
#include <../converter/VoidConverter.h>
#include <../converter/StringConverter.h>
#include <../converter/ProtocolBufferConverter.h>

#include <rsb/protocol/introspection/Hello.pb.h>
#include <rsb/protocol/introspection/Bye.pb.h>

#include "IntrospectionSender.h"

// Plugin-level helper stuff

namespace {

rsc::logging::LoggerPtr logger
    = rsc::logging::Logger::getLogger("rsb.introspection");

rsb::ParticipantConfig configSerialization;
rsb::ParticipantConfig configDeserialization;

rsb::introspection::IntrospectionSenderPtr sender;

template <typename WireType>
void addConverter
(rsb::converter::UnambiguousConverterMap<WireType>& selection,
 rsb::converter::Converter<WireType>*               converter,
 bool                                               forSerialization) {
    if (forSerialization) {
        selection.addConverter(converter->getDataType(),
                               typename rsb::converter::Converter<WireType>::Ptr(converter));
    } else {
        selection.addConverter(converter->getWireSchema(),
                               typename rsb::converter::Converter<WireType>::Ptr(converter));
    }
}

rsb::ParticipantConfig createConfig(const rsb::ParticipantConfig& defaultConfig,
                                    bool forSerialization) {
    rsb::ParticipantConfig config = defaultConfig;

    boost::shared_ptr< rsb::converter::UnambiguousConverterMap<std::string> > converterSelection
        (new rsb::converter::UnambiguousConverterMap<std::string>());
    addConverter(*converterSelection, new rsb::converter::VoidConverter(),
                 forSerialization);
    addConverter(*converterSelection, new rsb::converter::StringConverter(),
                 forSerialization);
    addConverter(*converterSelection,
                 new rsb::converter::ProtocolBufferConverter<rsb::protocol::introspection::Hello>(),
                 forSerialization);
    addConverter(*converterSelection,
                 new rsb::converter::ProtocolBufferConverter<rsb::protocol::introspection::Bye>(),
                 forSerialization);

    std::set<rsb::ParticipantConfig::Transport> transports
        = config.getTransports();
    for (std::set<rsb::ParticipantConfig::Transport>::const_iterator it
             = transports.begin(); it != transports.end(); ++it) {
        rsb::ParticipantConfig::Transport& transport
            = config.mutableTransport(it->getName());
        rsc::runtime::Properties options = transport.getOptions();
        options["converters"] = rsb::converter::ConverterSelectionStrategy<std::string>::Ptr(converterSelection);
        transport.setOptions(options);
    }

    return config;
}

void handleParticipantCreated(rsb::ParticipantPtr participant,
                              rsb::ParticipantPtr parent) {
    RSCDEBUG(logger, "Was notified of created participant " << participant);

    if (participant->getScope()->isSubScopeOf("/__rsb/introspection")
        || !participant->getConfig().isIntrospectionEnabled()) {
        RSCDEBUG(logger, "Ignoring created participant " << participant);
        return;
    }

    if (!sender) {
        RSCDEBUG(logger, "Creating introspection sender");
        configSerialization   = createConfig(rsb::getFactory().getDefaultParticipantConfig(), true);
        configDeserialization = createConfig(rsb::getFactory().getDefaultParticipantConfig(), false);
        sender.reset(new rsb::introspection::IntrospectionSender(configDeserialization, configSerialization));
    }

    sender->addParticipant(participant, parent);
}


void handleParticipantDestroyed(rsb::Participant* participant) {
    RSCDEBUG(logger, "Was notified of destroyed participant " << participant);

    if (participant->getScope()->isSubScopeOf("/__rsb/introspection")
        || !participant->getConfig().isIntrospectionEnabled()
        || !sender) {
        RSCDEBUG(logger, "Ignoring destroyed participant " << participant);
        return;
    }

    if (!sender->removeParticipant(*participant)) {
        sender.reset();
    }
}

boost::signals::connection participantCreatedConnection;
boost::signals::connection participantDestroyedConnection;

// External plugin interface

extern "C" {

    void RSC_PLUGIN_INIT_SYMBOL() {
        // Register converters.
        RSCDEBUG(logger, "Registering converters for introspection types");

        rsb::converter::converterRepository<std::string>()
            ->registerConverter(rsb::converter::ProtocolBufferConverter<rsb::protocol::introspection::Hello>::Ptr
                                (new rsb::converter::ProtocolBufferConverter<rsb::protocol::introspection::Hello>()));
        rsb::converter::converterRepository<std::string>()
            ->registerConverter(rsb::converter::ProtocolBufferConverter<rsb::protocol::introspection::Bye>::Ptr
                                (new rsb::converter::ProtocolBufferConverter<rsb::protocol::introspection::Bye>()));

        // Register participant creation hook.
        RSCDEBUG(logger, "Registering participant creation and destruction hooks");

        participantCreatedConnection
            = rsb::getFactory().getSignalParticipantCreated()
            ->connect(&handleParticipantCreated);
        participantDestroyedConnection
            = rsb::getFactory().getSignalParticipantDestroyed()
            ->connect(&handleParticipantDestroyed);
    }

    void RSC_PLUGIN_SHUTDOWN_SYMBOL() {
        RSCDEBUG(logger, "Unregistering participant creation and destruction hooks");

        participantCreatedConnection.disconnect();
        participantDestroyedConnection.disconnect();

        RSCDEBUG(logger, "Destroying introspection sender");

        sender.reset();
    }

}

}