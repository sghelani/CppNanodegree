#ifndef SYSTEM_PARSER_H
#define SYSTEM_PARSER_H

#include <map>
#include <regex>
#include <string>
#include <tuple>

using std::uint64_t;

namespace LinuxParser {
// Paths
const std::string kProcDirectory{"/proc/"};
const std::string kCmdlineFilename{"/cmdline"};
const std::string kCpuinfoFilename{"/cpuinfo"};
const std::string kStatusFilename{"/status"};
const std::string kStatFilename{"/stat"};
const std::string kUptimeFilename{"/uptime"};
const std::string kMeminfoFilename{"/meminfo"};
const std::string kVersionFilename{"/version"};
const std::string kOSPath{"/etc/os-release"};
const std::string kPasswordPath{"/etc/passwd"};

// file keywords
const std::string osName("PRETTY_NAME");
const std::string filterMemTotalString("MemTotal");
const std::string filterMemFreeString("MemFree");
const std::string filterMemBufferString("Buffers");
const std::string filterMemCacheString("Cached");
const std::string filterProcesses("processes");
const std::string filterRunningProcesses("procs_running");
const std::string filterProcMem("VmRSS");
const std::string filterUID("Uid");
const std::string filterCpu("cpu");
const std::string filterCores("cores");

// System
std::map<std::string, long> FetchSystemMemoryData();
float MemoryUtilization();
long UpTime();
std::vector<int> Pids();
int TotalProcesses();
int RunningProcesses();
std::pair<int, int> FetchNumProcesses();
std::string OperatingSystem();
std::string Kernel();
int NumCores();

// CPU
enum CPUStates {
  kUser_ = 0,
  kNice_,
  kSystem_,
  kIdle_,
  kIOwait_,
  kIRQ_,
  kSoftIRQ_,
  kSteal_,
  kGuest_,
  kGuestNice_
};

std::vector<uint64_t> GetTimeSpentInDiffStates(int cpuId);
std::vector<uint64_t> GetTimeSpentInDiffStates();
std::vector<uint64_t> FetchCPUTimeInDiffStates(std::string cpuId);

// Processes
std::vector<std::string> ProcessStatusValues(int pid);
float CpuUtilization(int pid);
std::string Command(int pid);
std::string Ram(int pid);
std::string Uid(int pid);
std::string User(int pid);
long int UpTime(int pid);
};  // namespace LinuxParser

#endif