#ifndef SYSTEM_H
#define SYSTEM_H

#include <map>
#include <string>
#include <vector>

#include "process.h"
#include "processor.h"

class System {
 public:
  /* Constructor */
  System();

  /* Getters */
  std::vector<Processor*>& Cpus();
  int GetNumCpus();
  std::vector<Process*>& Processes();
  float MemoryUtilization() const;
  long UpTime() const;
  int TotalProcesses() const;
  int RunningProcesses() const;
  std::string Kernel() const;
  std::string OperatingSystem() const;

  /* Setters */
  void OperatingSystem(std::string operatingSystem);
  void Kernel(std::string kernel);
  void TotalProcesses(int totalProcesses);
  void RunningProcesses(int runningProcesses);
  void UpTime(long upTime);
  void MemoryUtilization(float memortUtilization);
  void AddCpu(Processor* cpu);

  /* State Modifiers*/
  void RefreshAttributes();

 private:
  float memortUtilization_;
  long upTime_;
  int totalProcesses_;
  int runningProcesses_;
  std::string kernel_;
  std::string operatingSystem_;
  std::vector<Processor*> cpus_;
  std::vector<Process*> processes_;

  /* Below methods are for internal use only */
  std::map<int, Process*> MapPidToObj(vector<Process*>& processes);
  void RefreshProcesses();
  void RefreshCpus();
  vector<Process*> BuildProcessContainer();
};

#endif