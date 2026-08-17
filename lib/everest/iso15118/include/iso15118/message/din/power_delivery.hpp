// SPDX-License-Identifier: Apache-2.0
// Copyright 2026 Pionix GmbH and Contributors to EVerest
#pragma once

#include <iso15118/message/din/msg_data_types.hpp>

#include <everest/util/vector/fixed_vector.hpp>
#include <optional>
#include <variant>
#include <vector>

namespace iso15118::din::msg {

namespace data_types {
struct ProfileEntry {
    uint32_t charging_profile_entry_start; // Seconds from NOW
    int16_t charging_profile_entry_max_power;
};
using ChargingProfileList = everest::lib::util::fixed_vector<ProfileEntry, 24>;

struct ChargingProfile {
    SAScheduleTupleID sa_schedule_tuple_id;
    ChargingProfileList profile_entry;
};

struct DcEvPowerDeliveryParameter {
    DcEvStatus dc_ev_status;
    std::optional<bool> bulk_charging_complete{std::nullopt};
    bool charging_complete;
};
}; // namespace data_types

struct PowerDeliveryRequest {
    Header header;
    bool ready_to_charge_state;
    std::optional<data_types::ChargingProfile> charging_profile;
    std::optional<data_types::DcEvPowerDeliveryParameter> dc_ev_power_delivery_parameter;
};

struct PowerDeliveryResponse {
    Header header;
    data_types::ResponseCode response_code;
    std::variant<data_types::AcEvseStatus, data_types::DcEvseStatus> evse_status;
};

} // namespace iso15118::din::msg