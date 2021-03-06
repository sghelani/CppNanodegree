#ifndef PROCESSOR_H
#define PROCESSOR_H

#include <tuple>
#include <vector>

using std::tuple;
using std::uint64_t;
using std::vector;

class Processor {
 public:
  /* Constructor */
  Processor(int id);
  /* Getters */
  int GetCpuId() const;
  float Utilization();
  float CalculateUtilization();
  std::pair<uint64_t, uint64_t> PrevCpuValues();
  /* Setter */
  void PrevCpuValues(std::pair<uint64_t, uint64_t> pair);
  void RefreshProcessor();
  void Utilization(float utilization);

 private:
  int cpuId_;
  float utilization_{0.0f};
  uint64_t prevCpuIdleTime{0ULL};
  uint64_t prevCpuNonIdleTime{0ULL};
  std::pair<uint64_t, uint64_t> CalculateCPUIdleTime(vector<uint64_t>& cpuTime);
};

#endif
