// SPDX-FileCopyrightText: 2021 Sveriges Television AB
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "workers/QueuePointer.hh"

vivictpp::workers::QueuePointer::QueuePointer(int _value, int _range)
    : range(_range), value(_value % _range) {}

int vivictpp::workers::QueuePointer::getValue() { return value; }

vivictpp::workers::QueuePointer vivictpp::workers::QueuePointer::operator+(int other) {
  return QueuePointer(this->value + other, this->range);
}

vivictpp::workers::QueuePointer vivictpp::workers::QueuePointer::operator-(int other) {
  return QueuePointer(this->value - other + this->range, this->range);
}

bool vivictpp::workers::QueuePointer::operator==(const QueuePointer &other) {
  return this->value == other.value;
}

bool vivictpp::workers::QueuePointer::operator!=(const QueuePointer &other) {
  return this->value != other.value;
}

vivictpp::workers::QueuePointer &vivictpp::workers::QueuePointer::operator=(const int newValue) {
  value = newValue;
  return *this;
}

int vivictpp::workers::QueuePointer::distance(const vivictpp::workers::QueuePointer &other) {
  return (other.value + this->range - this->value) % this->range;
}

std::ostream &vivictpp::workers::operator<<(std::ostream &out, vivictpp::workers::QueuePointer qp) {
  out << "[value=" << qp.value << ",range=" << qp.range << "]";
  return out;
}
