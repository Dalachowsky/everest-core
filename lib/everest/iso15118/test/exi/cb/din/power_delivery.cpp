#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <iso15118/message/din/power_delivery.hpp>
#include <iso15118/message/din/variant.hpp>

#include "helper.hpp"

#include <vector>

using namespace iso15118;
namespace dt = din::msg::data_types;

SCENARIO("Ser/Deserialize din power delivery messages") {
    GIVEN("Deserialize power delivery req - minimal") {
        uint8_t doc_raw[] = {0x80, 0x9A, 0x02, 0x00, 0xB6, 0xC8, 0x81, 0xCE, 0xC2, 0x13, 0x4B, 0x51, 0x32, 0x60};

        const io::StreamInputView stream_view{doc_raw, sizeof(doc_raw)};

        din::msg::Variant variant(io::v2gtp::PayloadType::SAP, stream_view, false);

        THEN("It should be deserialized successfully") {
            REQUIRE(variant.get_type() == din::msg::Type::PowerDeliveryReq);

            const auto& msg = variant.get<din::msg::PowerDeliveryRequest>();
            const auto& header = msg.header;

            REQUIRE(header.session_id == std::array<uint8_t, 8>{0x02, 0xDB, 0x22, 0x07, 0x3B, 0x08, 0x4D, 0x2D});

            REQUIRE(msg.ready_to_charge_state == true);
        }
    }
    GIVEN("Deserialize power delivery req - all fields present") {
        uint8_t doc_raw[] = {0x80, 0x9A, 0x02, 0x00, 0xB6, 0xC8, 0x81, 0xCE, 0xC2, 0x13, 0x4B, 0x51, 0x32,
                             0x00, 0x04, 0x24, 0x07, 0x02, 0x41, 0x38, 0x41, 0x40, 0x0C, 0x81, 0x10, 0x00};

        const io::StreamInputView stream_view{doc_raw, sizeof(doc_raw)};

        din::msg::Variant variant(io::v2gtp::PayloadType::SAP, stream_view, false);

        THEN("It should be deserialized successfully") {
            REQUIRE(variant.get_type() == din::msg::Type::PowerDeliveryReq);

            const auto& msg = variant.get<din::msg::PowerDeliveryRequest>();
            const auto& header = msg.header;

            REQUIRE(header.session_id == std::array<uint8_t, 8>{0x02, 0xDB, 0x22, 0x07, 0x3B, 0x08, 0x4D, 0x2D});

            REQUIRE(msg.ready_to_charge_state == true);
            REQUIRE(msg.dc_ev_power_delivery_parameter.has_value());
            REQUIRE(msg.dc_ev_power_delivery_parameter->charging_complete == true);
            REQUIRE(msg.dc_ev_power_delivery_parameter->bulk_charging_complete.has_value());
            REQUIRE(msg.dc_ev_power_delivery_parameter->bulk_charging_complete.value() == true);
            REQUIRE(msg.dc_ev_power_delivery_parameter->dc_ev_status.ev_ready == true);
            REQUIRE(msg.dc_ev_power_delivery_parameter->dc_ev_status.ev_error_code == dt::DcEvErrorCode::NO_ERROR);
            REQUIRE(msg.dc_ev_power_delivery_parameter->dc_ev_status.ev_ress_soc == 50);

            REQUIRE(msg.charging_profile.has_value());
            const auto& profile = msg.charging_profile.value();
            REQUIRE(profile.sa_schedule_tuple_id == 1);
            REQUIRE(profile.profile_entry.size() == 1);
            REQUIRE(profile.profile_entry.at(0).charging_profile_entry_start == 3600);
            REQUIRE(profile.profile_entry.at(0).charging_profile_entry_max_power == 10000);
        }
    }
    GIVEN("Serialize power delivery res - AC") {

        const auto header = din::msg::Header{{0x02, 0xDB, 0x22, 0x07, 0x3B, 0x08, 0x4D, 0x2D}, std::nullopt};

        auto res = din::msg::PowerDeliveryResponse{};
        res.header = header;
        res.response_code = dt::ResponseCode::OK;
        auto status = dt::AcEvseStatus{};
        status.rcd = true;
        status.power_switch_closed = true;
        status.evse_notification = dt::EvseNotification::None;
        status.notification_max_delay = 0;
        res.evse_status = status;

        std::vector<uint8_t> expected = {0x80, 0x9A, 0x02, 0x00, 0xB6, 0xC8, 0x81, 0xCE, 0xC2,
                                         0x13, 0x4B, 0x51, 0x40, 0x00, 0x88, 0x00, 0x00, 0x00};

        THEN("It should be serialized successfully") {
            REQUIRE(serialize_helper(res) == expected);
        }
    }
    GIVEN("Serialize power delivery res - DC") {

        const auto header = din::msg::Header{{0x02, 0xDB, 0x22, 0x07, 0x3B, 0x08, 0x4D, 0x2D}, std::nullopt};

        auto res = din::msg::PowerDeliveryResponse{};
        res.header = header;
        res.response_code = dt::ResponseCode::OK;
        auto status = dt::DcEvseStatus{};
        status.evse_status_code = dt::DcEvseStatusCode::EVSE_Ready;
        status.evse_notification = dt::EvseNotification::None;
        status.notification_max_delay = 0;
        status.evse_isolation_status = dt::IsolationLevel::Invalid;
        res.evse_status = status;

        std::vector<uint8_t> expected = {0x80, 0x9A, 0x02, 0x00, 0xB6, 0xC8, 0x81, 0xCE, 0xC2,
                                         0x13, 0x4B, 0x51, 0x40, 0x04, 0x00, 0x40, 0x00, 0x00};

        THEN("It should be serialized successfully") {
            REQUIRE(serialize_helper(res) == expected);
        }
    }
}