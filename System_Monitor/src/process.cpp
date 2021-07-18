#include "process.h"

#include <unistd.h>

#include <cassert>
#include <cctype>
#include <sstream>
#include <string>
#include <vector>

#include "linux_parser.h"
using std::string;
using std::to_string;
using std::vector;

/* All attributes are initialized in the constructor */
Process::Process(int processId) {
  Pid(processId);
  Command(LinuxParser::Command(processId));
  User(LinuxParser::User(processId));
  PrevUtilizationValues({0ULL, 0ULL});
}
/* Refresh process attributes every second */
void Process::RefreshAttributes() {
  Ram(LinuxParser::Ram(Pid()));
  CpuUtilization(CalculateUtilization());
  UpTime(LinuxParser::UpTime(Pid()));
}

/* This formula has been derived from the stack overflow post */
float Process::CalculateUtilization() {
  vector<uint64_t> aggregateCpuTimes = LinuxParser::GetTimeSpentInDiffStates();
  vector<string> processStat = LinuxParser::ProcessStatusValues(Pid());
  uint64_t curProcTotal = stoull(processStat[13]) + stoull(processStat[14]);
  auto [prevProcTotal, prevCpuTotal] = PrevUtilizationValues();
  uint64_t curCpuTotal = 0ULL;
  for (uint64_t cpuStateTime : aggregateCpuTimes) {
    curCpuTotal += cpuStateTime;
  }
  PrevUtilizationValues({curProcTotal, curCpuTotal});
  float utilization = static_cast<float>(curProcTotal - prevProcTotal) /
                      (curCpuTotal - prevCpuTotal);

  return utilization;
}
/* getters */
int Process::Pid() const { return processId_; }
float Process::CpuUtilization() const { return cpuutilization_; }

/* Only display the 1st 50 characters of the command */
string Process::Command() const {
  if (command_.size() > commandCharDisplay) {
    return command_.substr(0, commandCharDisplay) + "...";
  }
  return command_;
}
string Process::Ram() const { return ram_; }
string Process::User() const { return user_; }
long int Process::UpTime() const { return upTime_; }
std::pair<uint64_t, uint64_t> Process::PrevUtilizationValues() {
  return {prevProcTotal_, prevCpuTotal_};
}

/* setters */
void Process::Pid(int pid) { processId_ = pid; }
void Process::User(std::string user) { user_ = user; }
void Process::Command(std::string command) { command_ = command; }
void Process::CpuUtilization(float cpuUtilization) {
  cpuutilization_ = cpuUtilization;
}
void Process::Ram(std::string ram) { ram_ = ram; }
void Process::UpTime(long uptime) { upTime_ = uptime; }
void Process::PrevUtilizationValues(std::pair<uint64_t, uint64_t> pair) {
  std::tie(prevProcTotal_, prevCpuTotal_) = pair;
}
