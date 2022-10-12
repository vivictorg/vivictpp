// SPDX-FileCopyrightText: 2022 Sveriges Television AB
//
// SPDX-License-Identifier: GPL-2.0-or-later


#ifndef SEEKING_HH_
#define SEEKING_HH_

#include "time/Time.hh"

#include <functional>

namespace vivictpp {

  typedef std::function<void(vivictpp::time::Time,bool)> SeekCallback;

}  // namespace vivictpp

#endif  // SEEKING_HH_
