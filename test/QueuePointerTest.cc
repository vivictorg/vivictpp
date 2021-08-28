// SPDX-FileCopyrightText: 2020 Sveriges Television AB
// SPDX-FileCopyrightText: 2021 Sveriges Television AB
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include <iostream>
#include "workers/QueuePointer.hh"

int main(int argc, char** argv) {
  int range = 20;
  QueuePointer qp1(0,range), qp2(0, range);
  std::cout << qp1 << "\n";
  
  std::cout << "qp1 + 2\n" << qp1 + 2 << "\n";
  std::cout << "qp1 + 23\n" << qp1 + 23 << "\n";
  qp1 = qp1 + 5;
  std::cout << "qp1 = qp1 + 5:\n" << qp1 << "\n";
  qp1 = qp1 - 12;
  std::cout  << "qp1 = qp1 - 12\n" << qp1 << "\n";
    
}
