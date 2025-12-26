#ifndef LOG_MANAGER_H
#define LOG_MANAGER_H

#include <functional>
#include <mutex>
#include <string>

class LogManager {
public:
  enum class LogLevel { Info, Warning, Error, Critical };
  using LogCallback = std::function<void(LogLevel, const std::string &)>;

  static void RegisterCallback(LogCallback cb);

  LogManager(const LogManager &) = delete;
  LogManager &operator=(const LogManager &) = delete;
  static void Init();
  static void Info(const std::string &message);
  static void Warning(const std::string &message);
  static void Error(const std::string &message);
  static void Critical(const std::string &message);

private:
  static void Write(const std::string &prefix, const std::string &message, bool error);
  static inline std::mutex logMutex;
  static void Notify(LogLevel level, const std::string &message);
  static inline std::vector<LogCallback> callbacks;
  static bool hasCritical;
  static bool initialized;
};

#endif
