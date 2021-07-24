#include "linux_parser.h"

#include <dirent.h>
#include <unistd.h>

#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

using std::map;
using std::stof;
using std::string;
using std::to_string;
using std::vector;

namespace fs = std::filesystem;

string LinuxParser::TransformInput(string input, MapChrToChr inpTrans) {
  string output = input;
  for (auto inpChar : inpTrans) {
    std::replace(output.begin(), output.end(), inpChar.first, inpChar.second);
  }
  return output;
}

bool LinuxParser::IsKeyFetched(MapStrToStr infoFetched, vector<string> keyArr) {
  for (auto key : keyArr) {
    if (infoFetched.find(key) == infoFetched.end()) return false;
  }
  return true;
}

std::istringstream LinuxParser::GetValueStream(std::string filePath) {
  string line;
  std::ifstream fileStream(filePath);
  std::istringstream lineStream("");
  if (fileStream.is_open()) {
    if (std::getline(fileStream, line)) {
      return std::istringstream(line);
    }
    fileStream.close();
  }
  return std::istringstream("");
}

MapStrToStr LinuxParser::FindValueByKey(vector<string> keyFilter,
                                        string filePath, MapChrToChr inpTrans) {
  string line, key, value;
  MapStrToStr keyValueMap;
  std::ifstream fileStream(filePath);

  if (fileStream.is_open()) {
    while (std::getline(fileStream, line)) {
      string newLine = TransformInput(line, inpTrans);
      std::istringstream lineStream(newLine);
      if (lineStream >> key >> value) {
        for (auto const keyName : keyFilter)
          if (key == keyName) keyValueMap.insert({key, value});
      }
    }
    fileStream.close();
  }
  return keyValueMap;
}
/* Fetches name of Operating System */
string LinuxParser::OperatingSystem() {
  MapChrToChr inpTrans = {{' ', '_'}, {'=', ' '}, {'"', ' '}};

  MapStrToStr keyValue = FindValueByKey({OsName}, kOSPath, inpTrans);
  if (!IsKeyFetched(keyValue, {OsName})) return "";

  string opsName = keyValue[OsName];
  std::replace(opsName.begin(), opsName.end(), '_', ' ');

  return opsName;
}
string LinuxParser::Kernel() {
  string os, kernel, version;

  std::istringstream linestream =
      GetValueStream(kProcDirectory + kVersionFilename);
  if (linestream.str().empty()) return "";

  linestream >> os >> version >> kernel;
  return kernel;
}

/* Process ids fetched using std::filesystem */
vector<int> LinuxParser::Pids() {
  const fs::path dir{kProcDirectory};
  vector<int> processIds;

  for (const auto &file : fs::directory_iterator(dir)) {
    if (file.is_directory()) {
      string dirName = file.path().stem().string();
      if (std::all_of(dirName.begin(), dirName.end(), isdigit))
        processIds.emplace_back(stoi(dirName));
    }
  }
  return processIds;
}

/* Computes Memory utilization of the whole system */
float LinuxParser::MemoryUtilization() {
  vector<string> keyArray = {MemTotalString, MemFreeString, MemBufferString,
                             MemCacheString};
  string filePath = kProcDirectory + kMeminfoFilename;
  MapChrToChr inTrans = {{':', ' '}};

  MapStrToStr memInfo = FindValueByKey(keyArray, filePath, inTrans);
  if (!IsKeyFetched(memInfo, keyArray)) return 0.0f;

  long memTotal = stol(memInfo[MemTotalString]);
  long memFree = stol(memInfo[MemFreeString]);
  long buffered = stol(memInfo[MemBufferString]);
  long cached = stol(memInfo[MemCacheString]);
  float utilization =
      static_cast<float>(memTotal - memFree - buffered - cached) / memTotal;

  return utilization;
}

/* Fetches upTime of the system in seconds */
long LinuxParser::UpTime() {
  string upTime = "0";

  std::istringstream stream = GetValueStream(kProcDirectory + kUptimeFilename);
  if (stream.str().empty()) return 0L;

  stream >> upTime;
  return static_cast<long>(std::stod(upTime));
}
std::vector<uint64_t> LinuxParser::FetchCPUTimeInDiffStates(string cpuId) {
  string line, key, value;
  vector<uint64_t> values;
  string filePath = kProcDirectory + kStatFilename;
  std::ifstream fileStream(filePath);

  if (fileStream.is_open()) {
    while (std::getline(fileStream, line)) {
      std::istringstream stream(line);
      string cpuIdStr = Cpu + cpuId;
      if (stream >> key && key == cpuIdStr) {
        while (stream >> value) values.emplace_back(stoull(value));
        break;
      }
    }
    fileStream.close();
  }

  return values;
}
/* Returns the aggregate data of all the cpus, overloaded with below function
 */
std::vector<uint64_t> LinuxParser::GetTimeSpentInDiffStates() {
  return FetchCPUTimeInDiffStates("");
}

/* Returns the time spent in various states for a particular cpu core */
std::vector<uint64_t> LinuxParser::GetTimeSpentInDiffStates(int cpuId) {
  return FetchCPUTimeInDiffStates(std::to_string(cpuId));
}

std::pair<int, int> LinuxParser::FetchNumProcesses() {
  vector<string> keyArray = {NumRunningProcesses, Processes};

  MapStrToStr procInfo =
      FindValueByKey(keyArray, kProcDirectory + kStatFilename);
  if (!IsKeyFetched(procInfo, keyArray)) return {0, 0};

  int numProcesses = stoi(procInfo[Processes]);
  int numRunningProcs = stoi(procInfo[NumRunningProcesses]);

  return {numProcesses, numRunningProcs};
}

/* Returns the command that triggered that process */
string LinuxParser::Command(int pid) {
  string filePath = kProcDirectory + to_string(pid) + kCmdlineFilename;

  std::istringstream stream = GetValueStream(filePath);
  if (stream.str().empty()) throw std::runtime_error(ErrorText);

  return stream.str();
}

/* eturns the memory utilization of each process in MB*/
string LinuxParser::Ram(int pid) {
  char ramInMb[9];
  MapChrToChr inTrans = {{':', ' '}};
  string filePath = kProcDirectory + to_string(pid) + kStatusFilename;

  MapStrToStr procInfo = FindValueByKey({ProcMem}, filePath, inTrans);
  if (!IsKeyFetched(procInfo, {ProcMem})) throw std::runtime_error(ErrorText);

  long memConsumption = stol(procInfo[ProcMem]);
  sprintf(ramInMb, "%.2f", static_cast<double>(memConsumption) / 1024);

  return ramInMb;
}

/* Used to fetch user id of the process */
string LinuxParser::Uid(int pid) {
  MapChrToChr inTrans = {{':', ' '}};
  string filePath = kProcDirectory + to_string(pid) + kStatusFilename;

  MapStrToStr procInfo = FindValueByKey({ProcUid}, filePath, inTrans);
  if (!IsKeyFetched(procInfo, {ProcUid})) throw std::runtime_error(ErrorText);

  return procInfo[ProcUid];
}
/* Used to fetch user of the process */
string LinuxParser::User(int pid) {
  string line, key, value;
  string uid = Uid(pid);
  string userName = "";
  std::ifstream fileStream(kPasswordPath);

  if (!fileStream.is_open()) throw std::runtime_error(ErrorText);

  while (std::getline(fileStream, line)) {
    std::stringstream strStream(line);
    vector<string> lineTokens;
    while (getline(strStream, key, ':')) lineTokens.emplace_back(key);
    if (lineTokens[2] == uid) userName = lineTokens[0];
  }
  fileStream.close();

  return userName;
}

/* Reads data from the /proc/{pid}/stat file */
// throw an exception here and catch everywhere else
vector<string> LinuxParser::ProcessStatusValues(int pid) {
  string key, line;
  vector<string> processStat;
  std::ifstream fileStream(kProcDirectory + to_string(pid) + kStatFilename);

  if (!fileStream.is_open()) throw std::runtime_error(ErrorText);
  while (std::getline(fileStream, line)) {
    std::istringstream stream(line);
    while (stream >> key) processStat.emplace_back(key);
  }
  fileStream.close();

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

/* The below code is another implementation of the process utilization which
 * can be used */
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
  int numCores = 0;

  if (fileStream.is_open()) {
    while (std::getline(fileStream, line)) {
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream stream(line);
      if (stream >> key1 >> key2) {
        if (key1 == Cpu && key2 == Cores && stream >> value)
          numCores = stoi(value);
      }
    }
    fileStream.close();
  }

  return numCores;
}
