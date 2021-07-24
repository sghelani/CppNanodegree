#include "processor.h"

#include <iostream>
#include <string>
#include <vector>

#include "linux_parser.h"
using namespace LinuxParser;

Processor::Processor(int id) : cpuId_(id) {}

/*Helper method which computes CPU idle time*/
std::pair<uint64_t, uint64_t> Processor::CalculateCPUIdleTime(
    vector<uint64_t>& cpuTime) {
  uint64_t idleTime = cpuTime[CPUStates::kIdle_] + cpuTime[CPUStates::kIOwait_];
  uint64_t nonIdleTime =
      cpuTime[CPUStates::kUser_] + cpuTime[CPUStates::kNice_] +
      cpuTime[CPUStates::kSystem_] + cpuTime[CPUStates::kIRQ_] +
      cpuTime[CPUStates::kSoftIRQ_] + cpuTime[CPUStates::kSteal_];

  return {idleTime, nonIdleTime};
}
/* Cpu utilization computed in reference to the values in the last second */
float Processor::CalculateUtilization() {
  std::vector<uint64_t> currentValues =
      LinuxParser::GetTimeSpentInDiffStates(GetCpuId());
  if (currentValues.empty()) return 0.0f;

  auto [prevIdle, prevNonIdle] = PrevCpuValues();
  auto [curIdle, curNonIdle] = CalculateCPUIdleTime(currentValues);
  uint64_t diffTotal = curIdle + curNonIdle - prevIdle - prevNonIdle;
  uint64_t diffIdle = curIdle - prevIdle;

  PrevCpuValues({curIdle, curNonIdle});

  return static_cast<float>(diffTotal - diffIdle) / diffTotal;
}

/* Getters */
float Processor::Utilization() { return utilization_; }
int Processor::GetCpuId() const { return cpuId_; }
std::pair<uint64_t, uint64_t> Processor::PrevCpuValues() {
  return {prevCpuIdleTime, prevCpuNonIdleTime};
}

/* Setters */
void Processor::PrevCpuValues(std::pair<uint64_t, uint64_t> pair) {
  std::tie(prevCpuIdleTime, prevCpuNonIdleTime) = pair;
}
void Processor::RefreshProcessor() { Utilization(CalculateUtilization()); }
void Processor::Utilization(float utilization) { utilization_ = utilization; }