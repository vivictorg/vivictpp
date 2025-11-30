// SPDX-FileCopyrightText: 2025 Gustav Grusell
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef VIVICTPP_IMGUI_ERRORDIALOG_HH_
#define VIVICTPP_IMGUI_ERRORDIALOG_HH_

#include "ErrorQueue.hh"
#include <memory>
#include <vector>

namespace vivictpp::imgui {

class ErrorDialog {
public:
  explicit ErrorDialog(vivictpp::ErrorQueue &errorQueue);

  // Draw the dialog. Returns true if dialog should remain open
  bool draw();

  // Check if we should open the dialog
  void checkForErrors();

private:
  vivictpp::ErrorQueue &errorQueue;
  std::vector<vivictpp::ErrorMessage> displayedErrors;
  bool isOpen{false};
  bool shouldOpen{false};
};

} // namespace vivictpp::imgui

#endif /* VIVICTPP_IMGUI_ERRORDIALOG_HH_ */
