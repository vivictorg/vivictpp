#include "ErrorQueue.hh"

namespace vivictpp {

void ErrorQueue::addError(const std::string &message,
                          const std::string &threadName) {
  std::lock_guard<std::mutex> lock(mutex);

  // Prevent unbounded growth
  if (errors.size() >= MAX_ERRORS) {
    errors.pop_front();
  }

  errors.emplace_back(message, threadName);
}

std::deque<ErrorMessage> ErrorQueue::getErrors() {
  std::lock_guard<std::mutex> lock(mutex);
  std::deque<ErrorMessage> result;
  result.swap(errors);
  return result;
}

bool ErrorQueue::hasErrors() const {
  std::lock_guard<std::mutex> lock(mutex);
  return !errors.empty();
}

void ErrorQueue::clear() {
  std::lock_guard<std::mutex> lock(mutex);
  errors.clear();
}

} // namespace vivictpp
