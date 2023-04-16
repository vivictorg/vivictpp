// SPDX-FileCopyrightText: 2021 Sveriges Television AB
//
// SPDX-License-Identifier: GPL-2.0-or-later


#ifndef LOGGING_LOGGING_HH
#define LOGGING_LOGGING_HH

#include "spdlog/spdlog.h"
#include "time/Time.hh"

namespace vivictpp {
namespace logging {

using Logger = std::shared_ptr<spdlog::logger>;

Logger getOrCreateLogger(std::string name);

void initializeLogging();

}  // namespace logging
}  // namespace vivictpp

#endif // LOGGING_LOGGING_HH
