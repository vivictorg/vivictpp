// SPDX-FileCopyrightText: 2021 Sveriges Television AB
//
// SPDX-License-Identifier: GPL-2.0-or-later


#ifndef LOGGING_HH_
#define LOGGING_HH_

#include "spdlog/spdlog.h"


namespace vivictpp {
namespace logging {

using Logger = std::shared_ptr<spdlog::logger>;

Logger getOrCreateLogger(std::string name);

}  // namespace logging
}  // namespace vivictpp

#endif  // LOGGING_HH_
