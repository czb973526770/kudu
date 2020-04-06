// Licensed to the Apache Software Foundation (ASF) under one
// or more contributor license agreements.  See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  The ASF licenses this file
// to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
// with the License.  You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.  See the License for the
// specific language governing permissions and limitations
// under the License.

#include "kudu/ranger/mini_ranger.h"

#include <string>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include "kudu/ranger/ranger.pb.h"
#include "kudu/util/status.h"
#include "kudu/util/test_macros.h"
#include "kudu/util/test_util.h"

using std::string;

namespace kudu {
namespace ranger {

class MiniRangerTest : public KuduTest {
 public:
  MiniRangerTest()
    : ranger_("127.0.0.1") {}
  void SetUp() override {
    ASSERT_OK(ranger_.Start());
  }

 protected:
  MiniRanger ranger_;
};

TEST_F(MiniRangerTest, TestGrantPrivilege) {
  PolicyItem item;
  item.first.emplace_back("testuser");
  item.second.emplace_back(ActionPB::ALTER);

  AuthorizationPolicy policy;
  policy.databases.emplace_back("foo");
  policy.tables.emplace_back("bar");
  policy.items.emplace_back(std::move(item));
  policy.name = "test1";

  ASSERT_OK(ranger_.AddPolicy(std::move(policy)));
}

TEST_F(MiniRangerTest, TestGrantSamePrivilegeAfterRestart) {
  PolicyItem item;
  item.first.emplace_back("testuser");
  item.second.emplace_back(ActionPB::ALTER);

  AuthorizationPolicy policy;
  policy.databases.emplace_back("foo");
  policy.tables.emplace_back("bar");
  policy.items.emplace_back(std::move(item));
  policy.name = "test1";

  ASSERT_OK(ranger_.AddPolicy(policy));

  ASSERT_OK(ranger_.Stop());
  ASSERT_OK(ranger_.Start());

  const string kExpectedError = "Another policy already exists for matching resource";

  Status s = ranger_.AddPolicy(std::move(policy));
  ASSERT_TRUE(s.IsRemoteError());
  ASSERT_STR_CONTAINS(s.ToString(), kExpectedError);
}

} // namespace ranger
} // namespace kudu