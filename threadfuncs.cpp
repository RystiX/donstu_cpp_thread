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

void Logger::writeLine(const std::string& msg) {
  std::lock_guard<std::mutex> lock(mutex_);
  file_ << msg;
  file_.flush();
  if (!file_) {
    std::cerr << "write failed: " << msg << "\n";
  }
}

pid_t getThreadID() {
  return static_cast<pid_t>(::syscall(SYS_gettid));
  //return GetCurrentThreadId();
}

void about() {
  std::cout << "std::thread example\n";
}

void funcThread(const ThreadArgs& args, Logger& logger) {
  for (int i = 0; i < COUNT_ITERATIONS; ++i) {
    std::ostringstream oss;

    oss << "[tag = " << args.tag
        << "] pid = "  << ::getpid()
        << " ppid = "  << ::getppid()
        << " tid = "   << getThreadID()
        << " iter = "  << i
        << "\n";
    logger.writeLine(oss.str());

    // imitation of useful work
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
}
