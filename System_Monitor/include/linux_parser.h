#ifndef SYSTEM_PARSER_H
#define SYSTEM_PARSER_H

#include <map>
#include <regex>
#include <sstream>
#include <string>
#include <tuple>
using std::uint64_t;
typedef std::map<std::string, std::string> MapStrToStr;
typedef std::map<char, char> MapChrToChr;

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
const std::string OsName("PRETTY_NAME");
const std::string MemTotalString("MemTotal");
const std::string MemFreeString("MemFree");
const std::string MemBufferString("Buffers");
const std::string MemCacheString("Cached");
const std::string Processes("processes");
const std::string NumRunningProcesses("procs_running");
const std::string ProcMem("VmRSS");
const std::string ProcUid("Uid");
const std::string Cpu("cpu");
const std::string Cores("cores");

const std::string ErrorText("Process data could not be read");

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

// Helper functions
MapStrToStr FindValueByKey(std::vector<std::string> keyFilter,
                           std::string filePath, MapChrToChr inpTrans = {});
std::string TransformInput(std::string input, MapChrToChr inpTrans);
std::istringstream GetValueStream(std::string filePath);
bool IsKeyFetched(MapStrToStr infoFetched, std::vector<std::string> keyArr);

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