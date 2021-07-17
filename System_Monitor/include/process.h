#ifndef PROCESS_H
#define PROCESS_H

#include <string>
/*
Basic class for Process representation
It contains relevant attributes as shown below
*/
class Process {
 public:
  /* constructor */
  Process(int processid);
  /* getters */
  int Pid() const;
  std::string User() const;
  std::string Command() const;
  float CpuUtilization() const;
  std::string Ram() const;
  long int UpTime() const;
  std::pair<uint64_t, uint64_t> PrevUtilizationValues();
  /* State Modifiers */
  void RefreshAttributes();

  /* Setters */
  void Pid(int pid);
  void User(std::string user);
  void Command(std::string command);
  void CpuUtilization(float cpuutilization);
  void Ram(std::string ram);
  void UpTime(long int uptime);
  void PrevUtilizationValues(std::pair<uint64_t, uint64_t> pair);

  /* static attributes */
  static constexpr int commandCharDisplay{40};

 private:
  int processId_;
  std::string command_;
  std::string user_;
  std::string ram_;
  float cpuutilization_;
  long int upTime_;
  uint64_t prevProcTotal_;
  uint64_t prevCpuTotal_;
  float CalculateUtilization();
};

#endif