// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#include "ListDeclaredTasks.hpp"
#include "Error.hpp"
#include "Settings.hpp"
#include "json/ParserOutputStream.hxx"
#include "net/http/Progress.hpp"
#include "lib/curl/CoStreamRequest.hxx"
#include "lib/curl/Easy.hxx"
#include "lib/curl/Setup.hxx"
#include "lib/fmt/ToBuffer.hxx"
#include "co/Task.hxx"

#include <boost/json.hpp>

namespace WeGlide {

static auto
tag_invoke(boost::json::value_to_tag<TaskInfo>,
           const boost::json::value &jv)
{
  const auto &json = jv.as_object();

  TaskInfo info;
  info.id = json.at("id").to_number<uint_least64_t>();
  info.name = json.at("name").as_string();
  info.user_name = json.at("user").at("name").as_string();
  info.distance = json.at("distance").to_number<double>();
  return info;
}



Co::Task<std::vector<TaskInfo>>
ListDeclaredTasks(CurlGlobal &curl, const WeGlideSettings &settings,
                  ProgressListener &progress)
{
  const auto url = FmtBuffer<256>("{}/task/declaration",
                                  settings.default_url);

  CurlEasy easy{url};
  Curl::Setup(easy);
  const Net::ProgressAdapter progress_adapter{easy, progress};

  Json::ParserOutputStream parser;
  const auto response =
    co_await Curl::CoStreamRequest(curl, std::move(easy), parser);
  auto body = parser.Finish();

  if (response.status != 200)
    throw ResponseToException(response.status, body);

  co_return boost::json::value_to<std::vector<TaskInfo>>(body);
}

} // namespace WeGlide
