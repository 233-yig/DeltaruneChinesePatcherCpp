#include "LogManager.h"
#include <chrono>
#include <ctime>
#include <fstream>
#include <functional>
#include <iomanip>
#include <mutex>
#include <sstream>
#include <tinyfiledialogs.h>

bool LogManager::initialized = false;
bool LogManager::hasCritical = false;

void LogManager::Init() {
  std::vector<std::string> logFileList = {"log.txt", "error.txt"};
  for (auto &fileName : logFileList) {
    std::ofstream file(fileName, std::ios::trunc);
    if (file.is_open()) {
      std::time_t now = std::time(nullptr);
      std::tm *tm = std::localtime(&now);

      std::ostringstream oss;
      oss << std::put_time(tm, "%Y-%m-%d %H:%M:%S");

      file << "========================================\n";
      file << fileName + " started at " << oss.str() << "\n";
      file << "========================================\n\n";
      file.close();
    }
  }
  initialized = true;
}

void LogManager::Info(const std::string &message) {
  Write("-- [Info] ", message, false);
  Notify(LogLevel::Info, message);
}

void LogManager::Warning(const std::string &message) {
  Write("-> [Warning] ", message, true);
  Notify(LogLevel::Warning, message);
}

void LogManager::Error(const std::string &message) {
  Write("!! [Error] ", message, true);
  Notify(LogLevel::Error, message);
}

void LogManager::Critical(const std::string &message) {
  if (hasCritical)
    return;
  hasCritical = true;
  Write("@@ [Fatal Error] ", message, true);
  tinyfd_messageBox("Fatal Error", (message + "\nProgram will abort.").c_str(),
                    "ok", "error", 1);
  Notify(LogLevel::Critical, message);
  exit(1);
}

void LogManager::Write(const std::string &prefix, const std::string &message,
                       bool error) {
  if (!initialized) {
    Init();
  }
  auto now = std::chrono::system_clock::now();
  auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                now.time_since_epoch()) %
            1000;
  std::time_t nowTime = std::chrono::system_clock::to_time_t(now);
  std::lock_guard<std::mutex> lock(logMutex);
  std::tm *tmPtr = std::localtime(&nowTime);

  std::vector<std::string> logFileList = {"log.txt"};
  if (error)
    logFileList.push_back("error.txt");
  for (auto &fileName : logFileList) {
    std::ofstream logFile(fileName, std::ios::app);
    if (logFile.is_open()) {
      logFile << prefix << std::put_time(tmPtr, "%Y-%m-%d %H:%M:%S") << "."
              << std::setfill('0') << std::setw(3) << ms.count() << " "
              << message << std::endl;
    }
  }
}

void LogManager::RegisterCallback(LogCallback cb) {
  callbacks.push_back(std::move(cb));
}

void LogManager::Notify(LogLevel level, const std::string &message) {
  for (auto &cb : callbacks) {
    cb(level, message);
  }
}
