// SPDX-License-Identifier: Apache-2.0
// Copyright 2026 Pionix GmbH and Contributors to EVerest
#include <iso15118/message/din/power_delivery.hpp>

#include <iso15118/detail/variant_access.hpp>

#include <cbv2g/din/din_msgDefDecoder.h>
#include <cbv2g/din/din_msgDefEncoder.h>

#include <iso15118/detail/helper.hpp>
#include <variant>

namespace iso15118::din::msg {

template <> void convert(const struct din_PowerDeliveryReqType& in, PowerDeliveryRequest& out) {
    out.ready_to_charge_state = in.ReadyToChargeState;

    if (in.ChargingProfile_isUsed) {
        auto& charging_profile = out.charging_profile.emplace();
        charging_profile.sa_schedule_tuple_id = in.ChargingProfile.SAScheduleTupleID;
        for (size_t i = 0; i < in.ChargingProfile.ProfileEntry.arrayLen; i++) {
            const auto& entry = in.ChargingProfile.ProfileEntry.array[i];
            auto entry_out = data_types::ProfileEntry{};
            entry_out.charging_profile_entry_start = entry.ChargingProfileEntryStart;
            entry_out.charging_profile_entry_max_power = entry.ChargingProfileEntryMaxPower;

            charging_profile.profile_entry.push_back(entry_out);
        }
    }

    if (in.DC_EVPowerDeliveryParameter_isUsed) {
        const auto& param = in.DC_EVPowerDeliveryParameter;
        auto& param_out = out.dc_ev_power_delivery_parameter.emplace();
        convert(param.DC_EVStatus, param_out.dc_ev_status);
        CB2CPP_ASSIGN_IF_USED(param.BulkChargingComplete, param_out.bulk_charging_complete);
        param_out.charging_complete = param.ChargingComplete;
    }
}

template <>
void insert_type(VariantAccess& va, const struct din_PowerDeliveryReqType& in,
                 const struct din_MessageHeaderType& header) {
    va.insert_type<PowerDeliveryRequest>(in, header);
}

template <> void convert(const PowerDeliveryResponse& in, struct din_PowerDeliveryResType& out) {
    init_din_PowerDeliveryResType(&out);

    cb_convert_enum(in.response_code, out.ResponseCode);
    if (std::holds_alternative<data_types::AcEvseStatus>(in.evse_status)) {
        const auto& status = std::get<data_types::AcEvseStatus>(in.evse_status);
        convert(status, out.AC_EVSEStatus);
        CB_SET_USED(out.AC_EVSEStatus);
    } else if (std::holds_alternative<data_types::DcEvseStatus>(in.evse_status)) {
        const auto& status = std::get<data_types::DcEvseStatus>(in.evse_status);
        convert(status, out.DC_EVSEStatus);
        CB_SET_USED(out.DC_EVSEStatus);
    }
}

template <> int serialize_to_exi(const PowerDeliveryResponse& in, exi_bitstream_t& out) {

    din_exiDocument doc;
    init_din_exiDocument(&doc);
    init_din_BodyType(&doc.V2G_Message.Body);

    convert(in.header, doc.V2G_Message.Header);

    CB_SET_USED(doc.V2G_Message.Body.PowerDeliveryRes);
    convert(in, doc.V2G_Message.Body.PowerDeliveryRes);

    return encode_din_exiDocument(&out, &doc);
}

template <> size_t serialize(const PowerDeliveryResponse& in, const io::StreamOutputView& out) {
    return serialize_helper(in, out);
}

} // namespace iso15118::din::msg