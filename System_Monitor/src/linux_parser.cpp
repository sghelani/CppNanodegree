#include "linux_parser.h"

#include <dirent.h>
#include <unistd.h>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using std::map;
using std::stof;
using std::string;
using std::to_string;
using std::vector;

namespace fs = std::filesystem;

/* Fetches name of Operating System */
string LinuxParser::OperatingSystem() {
  string line;
  string key;
  string value;
  std::ifstream filestream(kOSPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), '=', ' ');
      std::replace(line.begin(), line.end(), '"', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == osName) {
          std::replace(value.begin(), value.end(), '_', ' ');
          return value;
        }
      }
    }
    filestream.close();
  }
  return value;
}
/* Fetches name of kernel */
string LinuxParser::Kernel() {
  string os, kernel, version;
  string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> version >> kernel;
    stream.close();
  }
  return kernel;
}

/* Process ids fetched using std::filesystem */
vector<int> LinuxParser::Pids() {
  const fs::path dir{kProcDirectory};
  vector<int> processIds;
  for (const auto& file : fs::directory_iterator(dir)) {
    if (file.is_directory()) {
      string dirName = file.path().stem().string();
      if (std::all_of(dirName.begin(), dirName.end(), isdigit))
        processIds.emplace_back(stoi(dirName));
    }
  }
  return processIds;
}

/* Used to fetch memory usage data of whole system from file /proc/meminfo */
map<string, long> LinuxParser::FetchSystemMemoryData() {
  string line, key, value;
  std::ifstream fileStream(kProcDirectory + kMeminfoFilename);
  map<string, long> memoryInfo;
  if (fileStream.is_open()) {
    while (std::getline(fileStream, line)) {
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream linestream(line);
      if (linestream >> key >> value) memoryInfo.insert({key, stol(value)});
    }
    fileStream.close();
  }
  return memoryInfo;
}

/* Computes Memory utilization of the whole system */
float LinuxParser::MemoryUtilization() {
  map<string, long> memoryInfo = FetchSystemMemoryData();
  long memTotal = memoryInfo[filterMemTotalString];
  long memFree = memoryInfo[filterMemFreeString];
  long buffered = memoryInfo[filterMemBufferString];
  long cached = memoryInfo[filterMemCacheString];
  float utilization =
      static_cast<float>(memTotal - memFree - buffered - cached) / memTotal;
  return utilization;
}

/* Fetches upTime of the system in seconds */
long LinuxParser::UpTime() {
  string line, upTime;
  std::ifstream fileStream(kProcDirectory + kUptimeFilename);
  if (fileStream.is_open()) {
    std::getline(fileStream, line);
    std::istringstream stream(line);
    stream >> upTime;
    fileStream.close();
  }
  return static_cast<long>(std::stod(upTime));
}
std::vector<uint64_t> LinuxParser::FetchCPUTimeInDiffStates(string cpuId) {
  string line, key, value;
  vector<uint64_t> values;
  std::ifstream fileStream(kProcDirectory + kStatFilename);
  if (fileStream.is_open()) {
    while (std::getline(fileStream, line)) {
      std::istringstream stream(line);
      string cpuIdStr = filterCpu + cpuId;
      if (stream >> key && key == cpuIdStr) {
        while (stream >> value) values.emplace_back(stoull(value));
        break;
      }
    }
    fileStream.close();
  }
  return values;
}
/* Returns the aggregate data of all the cpus, overloaded with below function */
std::vector<uint64_t> LinuxParser::GetTimeSpentInDiffStates() {
  return FetchCPUTimeInDiffStates("");
}
/* Returns the time spent in various states for a particular cpu core */
std::vector<uint64_t> LinuxParser::GetTimeSpentInDiffStates(int cpuId) {
  return FetchCPUTimeInDiffStates(std::to_string(cpuId));
}

/* Returns the total count of processes and active processes in the system */
std::pair<int, int> LinuxParser::FetchNumProcesses() {
  string line, key, value;
  int numProcesses, numRunningProcs;
  std::ifstream fileStream(kProcDirectory + kStatFilename);
  if (fileStream.is_open()) {
    while (std::getline(fileStream, line)) {
      std::istringstream linestream(line);
      if (linestream >> key >> value) {
        if (key == filterRunningProcesses)
          numRunningProcs = std::stoi(value);
        else if (key == filterProcesses)
          numProcesses = std::stoi(value);
      }
    }
    fileStream.close();
  }
  return {numProcesses, numRunningProcs};
}

/* Returns the command that triggered that process */
string LinuxParser::Command(int pid) {
  string line;
  std::ifstream fileStream(kProcDirectory + to_string(pid) + kCmdlineFilename);
  if (fileStream.is_open()) {
    if (std::getline(fileStream, line)) return line;
    fileStream.close();
  }
  return "";
}

/* Returns the memory utilization of each process in MB*/
string LinuxParser::Ram(int pid) {
  string line, key, value;
  char ramInMb[9];
  long memConsumption = 0L;
  std::ifstream fileStream(kProcDirectory + to_string(pid) + kStatusFilename);
  if (fileStream.is_open()) {
    while (std::getline(fileStream, line)) {
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream stream{line};
      if (stream >> key >> value)
        if (key == filterProcMem) memConsumption = stol(value);
    }
    fileStream.close();
  }
  sprintf(ramInMb, "%.2f", static_cast<double>(memConsumption) / 1024);
  return ramInMb;
}

/* Used to fetch user id of the process */
string LinuxParser::Uid(int pid) {
  string line, key, value;
  std::ifstream fileStream(kProcDirectory + to_string(pid) + kStatusFilename);
  if (fileStream.is_open()) {
    while (std::getline(fileStream, line)) {
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream stream{line};
      if (stream >> key >> value)
        if (key == filterUID) return value;
    }
    fileStream.close();
  }
  return "";
}
/* Used to fetch user of the process */
string LinuxParser::User(int pid) {
  string line, key, value;
  string uid = Uid(pid);
  std::ifstream fileStream(kPasswordPath);
  if (fileStream.is_open()) {
    while (std::getline(fileStream, line)) {
      std::stringstream strStream(line);
      vector<string> lineTokens;
      while (getline(strStream, key, ':')) lineTokens.emplace_back(key);
      if (lineTokens[2] == uid) return lineTokens[0];
    }
    fileStream.close();
  }
  return "";
}

/* Reads data from the /proc/{pid}/stat file */
vector<string> LinuxParser::ProcessStatusValues(int pid) {
  string key, line;
  vector<string> processStat;
  std::ifstream fileStream(kProcDirectory + to_string(pid) + kStatFilename);
  if (fileStream.is_open()) {
    while (std::getline(fileStream, line)) {
      std::istringstream stream(line);
      while (stream >> key) processStat.emplace_back(key);
    }
    fileStream.close();
  }
  return processStat;
}

/* Returns the up time of each process */
long int LinuxParser::UpTime(int pid) {
  vector<string> processStat = ProcessStatusValues(pid);
  long upTimeTicks = stol(processStat[21]);
  long upTime = static_cast<long>(LinuxParser::UpTime() -
                                  (upTimeTicks / sysconf(_SC_CLK_TCK)));
  return upTime;
}

/* The below code is another implementation of the process utilization which can
 * be used */
float LinuxParser::CpuUtilization(int pid) {
  vector<string> processStat = ProcessStatusValues(pid);
  uint64_t utime = stoull(processStat[13]);
  uint64_t stime = stoull(processStat[14]);
  uint64_t cutime = stoull(processStat[15]);
  uint64_t cstime = stoull(processStat[16]);
  uint64_t totalTime = utime + stime + cutime + cstime;

  float utilization =
      static_cast<float>(totalTime / sysconf(_SC_CLK_TCK)) / UpTime(pid);
  return utilization;
}

/*  Fetches the number of cpu cores in the system */
int LinuxParser::NumCores() {
  string line, key1, key2, value;
  std::ifstream fileStream{kProcDirectory + kCpuinfoFilename};
  if (fileStream.is_open()) {
    while (std::getline(fileStream, line)) {
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream stream(line);
      if (stream >> key1 >> key2) {
        if (key1 == filterCpu && key2 == filterCores && stream >> value)
          return stoi(value);
      }
    }
    fileStream.close();
  }
  return 0;
}
