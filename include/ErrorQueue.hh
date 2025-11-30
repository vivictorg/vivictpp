#ifndef VIVICTPP_ERRORQUEUE_HH
#define VIVICTPP_ERRORQUEUE_HH

#include <chrono>
#include <deque>
#include <mutex>
#include <string>

namespace vivictpp {

struct ErrorMessage {
  std::string message;
  std::string threadName;
  std::chrono::system_clock::time_point timestamp;

  ErrorMessage(std::string msg, std::string thread)
      : message(std::move(msg)), threadName(std::move(thread)),
        timestamp(std::chrono::system_clock::now()) {}
};

class ErrorQueue {
public:
  ErrorQueue() = default;

  // Add an error from any thread
  void addError(const std::string &message, const std::string &threadName);

  // Get all pending errors (called from UI thread)
  std::deque<ErrorMessage> getErrors();

  // Check if there are any errors
  bool hasErrors() const;

  // Clear all errors
  void clear();

private:
  mutable std::mutex mutex;
  std::deque<ErrorMessage> errors;
  static constexpr size_t MAX_ERRORS = 100; // Prevent unbounded growth
};

} // namespace vivictpp

#endif // VIVICTPP_ERRORQUEUE_HH
