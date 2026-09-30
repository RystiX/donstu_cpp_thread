#pragma once
#include <atomic>
#include <string>
#include <mutex>
#include <fstream>

// count of threads and iterations
constexpr int COUNT_THREADS    = 4;
constexpr int COUNT_ITERATIONS = 3;
extern std::atomic<int> counter;

// args for thread
struct ThreadArgs {
  int         id;
  std::string tag;
};

// common resources in separate class
class Logger {
public:
  explicit Logger(const std::string& filename);
  ~Logger();

  // write line with mutex
  bool writeLine(const std::string& msg);

  // block copy and move
  Logger(const Logger&)            = delete;
  Logger& operator=(const Logger&) = delete;

private:
  std::ofstream    file_;
  std::mutex       mutex_;
};

// function for thread
void funcThread(const ThreadArgs& args, Logger& logger);

// get system TID for current linux thread
pid_t getThreadID();

// healline of software
void about();
