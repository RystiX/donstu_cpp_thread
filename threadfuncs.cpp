// threadfuncs.cpp
#include "threadfuncs.h"

#include <iostream>
#include <thread>
#include <chrono>
#include <sstream>
#include <unistd.h>
#include <syscall.h>
//#include <windows.h>
#include <sys/types.h>

// Global atomic counter
std::atomic<int> counter{0};

Logger::Logger(const std::string& filename)
  : file_(filename, std::ios::out | std::ios::trunc)
{
  if (!file_.is_open()) {
    throw std::runtime_error("Cannot open log file: " + filename);
  }
}

Logger::~Logger() {
  // std::ofstream close file here automatically
}

bool Logger::writeLine(const std::string& msg)
{
    std::lock_guard<std::mutex> lock(mutex_);

    if (!file_)
        return false;

    file_ << msg << "\n";

    return static_cast<bool>(file_);
}

pid_t getThreadID() {
  return static_cast<pid_t>(::syscall(SYS_gettid));
  //return GetCurrentThreadId();
}

void about() {
  std::cout << "std::thread example\n";
}

void funcThread(const ThreadArgs& args, Logger& logger) {
  for (int i = 0; i < 100000; ++i) {
    counter++;
  }

  for (int i = 0; i < COUNT_ITERATIONS; ++i) {
    std::ostringstream oss;

    oss << "[tag = " << args.tag
    << "] pid = " << ::getpid()
    << " ppid = " << ::getppid()
    << " std::thread::id = " << std::this_thread::get_id()
    << " sys_tid = " << getThreadID()
    << " iter = " << i;
    logger.writeLine(oss.str());

    // imitation of useful work
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
}
