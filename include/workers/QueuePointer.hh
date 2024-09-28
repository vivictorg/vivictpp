// SPDX-FileCopyrightText: 2020 Sveriges Television AB
// SPDX-FileCopyrightText: 2021 Sveriges Television AB
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef WORKERS_QUEUEPOINTER_HH
#define WORKERS_QUEUEPOINTER_HH

#include <iostream>

namespace vivictpp {
namespace workers {

class QueuePointer {
public:
  QueuePointer(int _value, int _range);
  QueuePointer operator+(int other);
  QueuePointer operator-(int other);
  QueuePointer &operator=(const int newValue);
  int distance(const QueuePointer &other);
  bool operator==(const QueuePointer &other);
  bool operator!=(const QueuePointer &other);
  operator int() { return value; }
  int getValue();
  friend std::ostream &operator<<(std::ostream &out, QueuePointer qp);

private:
  int range;
  int value;
};

std::ostream &operator<<(std::ostream &out, QueuePointer qp);
} // namespace workers
} // namespace vivictpp

#endif // WORKERS_QUEUEPOINTER_HH
