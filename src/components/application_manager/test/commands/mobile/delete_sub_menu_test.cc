/*
 * Copyright (c) 2016, Ford Motor Company
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following
 * disclaimer in the documentation and/or other materials provided with the
 * distribution.
 *
 * Neither the name of the Ford Motor Company nor the names of its contributors
 * may be used to endorse or promote products derived from this software
 * without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdint.h>
#include <string>

#include "application_manager/commands/mobile/delete_sub_menu_request.h"

#include "gtest/gtest.h"
#include "application_manager/commands/command_request_test.h"
#include "application_manager/mock_application_manager.h"
#include "application_manager/mock_application.h"
#include "application_manager/mock_message_helper.h"
#include "application_manager/event_engine/event.h"
#include "application_manager/mock_hmi_interface.h"

namespace test {
namespace components {
namespace commands_test {
namespace mobile_commands_test {
namespace delete_sub_menu_request {

using ::testing::_;
using ::testing::Mock;
using ::testing::Return;
using ::testing::ReturnRef;
namespace am = ::application_manager;
using am::commands::DeleteSubMenuRequest;
using am::commands::MessageSharedPtr;
using am::event_engine::Event;
using am::MockHmiInterfaces;
using am::MockMessageHelper;

typedef SharedPtr<DeleteSubMenuRequest> AddSubMenuPtr;

namespace {
const uint32_t kConnectionKey = 2u;
}  // namespace

class DeleteSubMenuRequestTest
    : public CommandRequestTest<CommandsTestMocks::kIsNice> {
 public:
  DeleteSubMenuRequestTest()
      : mock_message_helper_(*MockMessageHelper::message_helper_mock()) {}
  MockMessageHelper& mock_message_helper_;
  sync_primitives::Lock lock_;
};

TEST_F(DeleteSubMenuRequestTest, OnEvent_UI_UNSUPPORTED_RESOURCE) {
  MessageSharedPtr msg = CreateMessage(smart_objects::SmartType_Map);
  (*msg)[am::strings::params][am::strings::connection_key] = kConnectionKey;
  (*msg)[am::strings::msg_params][am::strings::menu_id] = 10u;

  utils::SharedPtr<DeleteSubMenuRequest> command =
      CreateCommand<DeleteSubMenuRequest>(msg);

  MockAppPtr mock_app = CreateMockApp();
  ON_CALL(app_mngr_, application(kConnectionKey))
      .WillByDefault(Return(mock_app));
  ON_CALL(*mock_app, app_id()).WillByDefault(Return(kConnectionKey));
  EXPECT_CALL(*mock_app, RemoveSubMenu(_));

  MessageSharedPtr ev_msg = CreateMessage(smart_objects::SmartType_Map);
  (*ev_msg)[am::strings::params][am::hmi_response::code] =
      hmi_apis::Common_Result::UNSUPPORTED_RESOURCE;
  (*ev_msg)[am::strings::msg_params][am::strings::info] = "info";

  Event event(hmi_apis::FunctionID::UI_DeleteSubMenu);
  event.set_smart_object(*ev_msg);

  MockHmiInterfaces hmi_interfaces;
  ON_CALL(app_mngr_, hmi_interfaces()).WillByDefault(ReturnRef(hmi_interfaces));
  EXPECT_CALL(hmi_interfaces,
              GetInterfaceState(am::HmiInterfaces::HMI_INTERFACE_UI))
      .WillOnce(Return(am::HmiInterfaces::STATE_AVAILABLE));

  EXPECT_CALL(mock_message_helper_,
              HMIToMobileResult(hmi_apis::Common_Result::UNSUPPORTED_RESOURCE))
      .WillOnce(Return(mobile_apis::Result::UNSUPPORTED_RESOURCE));

  am::CommandsMap commands_map;
  smart_objects::SmartObject commands_msg(smart_objects::SmartType_Map);
  commands_map.insert(std::pair<uint32_t, SmartObject*>(1u, &commands_msg));
  DataAccessor<am::CommandsMap> accessor(commands_map, lock_);
  EXPECT_CALL(*mock_app, commands_map())
      .WillOnce(Return(accessor))
      .WillOnce(Return(accessor));

  MessageSharedPtr ui_command_result;
  EXPECT_CALL(
      app_mngr_,
      ManageMobileCommand(_, am::commands::Command::CommandOrigin::ORIGIN_SDL))
      .WillOnce(DoAll(SaveArg<0>(&ui_command_result), Return(true)));

  command->on_event(event);

  EXPECT_EQ((*ui_command_result)[am::strings::msg_params][am::strings::success]
                .asBool(),
            true);
  EXPECT_EQ(
      (*ui_command_result)[am::strings::msg_params][am::strings::result_code]
          .asInt(),
      static_cast<int32_t>(hmi_apis::Common_Result::UNSUPPORTED_RESOURCE));
  if ((*ui_command_result)[am::strings::msg_params].keyExists(
          am::strings::info)) {
    EXPECT_FALSE(
        (*ui_command_result)[am::strings::msg_params][am::strings::info]
            .asString()
            .empty());
  }
  Mock::VerifyAndClearExpectations(&mock_message_helper_);
}

}  // namespace delete_sub_menu_request
}  // namespace mobile_commands_test
}  // namespace commands_test
}  // namespace components
}  // namespace test