#include "process.h"

#include <unistd.h>

#include <cctype>
#include <numeric>
#include <sstream>
#include <string>
#include <vector>

#include "linux_parser.h"
using std::string;
using std::to_string;
using std::vector;

/* All attributes are initialized in the constructor */
Process::Process(int processId) : processId_(processId) {}
/* Refresh process attributes every second */
void Process::RefreshAttributes() {
  /* The below two attributes dont change with time */
  if (Command().empty()) Command(LinuxParser::Command(Pid()));
  if (User().empty()) User(LinuxParser::User(Pid()));
  /* Gets updated every second */
  Ram(LinuxParser::Ram(Pid()));
  CpuUtilization(CalculateUtilization());
  UpTime(LinuxParser::UpTime(Pid()));
}

/* This formula has been derived from the stack overflow post */
float Process::CalculateUtilization() {
  vector<uint64_t> aggrCpuTimes = LinuxParser::GetTimeSpentInDiffStates();
  vector<string> procStat = LinuxParser::ProcessStatusValues(Pid());
  auto [prevProcTotal, prevCpuTotal] = PrevUtilizationValues();

  uint64_t curProcTotal = stoull(procStat[13]) + stoull(procStat[14]);
  uint64_t curCpuTotal =
      std::accumulate(aggrCpuTimes.begin(), aggrCpuTimes.end(), 0ULL);
  float utilization = static_cast<float>(curProcTotal - prevProcTotal) /
                      (curCpuTotal - prevCpuTotal);
  /* The new becomes the old*/
  PrevUtilizationValues({curProcTotal, curCpuTotal});

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
utilPair Process::PrevUtilizationValues() {
  return {prevProcTotal_, prevCpuTotal_};
}

/* setters */
void Process::Pid(int pid) { processId_ = pid; }
void Process::User(std::string user) { user_ = user; }
void Process::Command(std::string command) { command_ = command; }
void Process::CpuUtilization(float cpuUtil) { cpuutilization_ = cpuUtil; }
void Process::Ram(std::string ram) { ram_ = ram; }
void Process::UpTime(long uptime) { upTime_ = uptime; }
void Process::PrevUtilizationValues(utilPair pair) {
  std::tie(prevProcTotal_, prevCpuTotal_) = pair;
}
