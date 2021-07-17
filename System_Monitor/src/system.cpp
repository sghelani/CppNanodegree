#include "system.h"

#include <unistd.h>

#include <algorithm>
#include <cstddef>
#include <string>
#include <utility>
#include <vector>

#include "linux_parser.h"
#include "process.h"
#include "processor.h"

using std::size_t;
using std::string;
using std::vector;

/* Initializing attribs that dont need to be updated with every refresh */
System::System() {
  /* Data for each cpu core is stored and maintained */
  for (int i = 0; i < LinuxParser::NumCores(); i++) {
    AddCpu(new Processor(i));
  }
  Kernel(LinuxParser::Kernel());
  OperatingSystem(LinuxParser::OperatingSystem());
}
/* Refresh cpu data so that latest cpu utilization values can be fetched */
void System::RefreshCpus() {
  vector<Processor*>& cpus = Cpus();
  for (Processor* p : cpus) {
    p->RefreshProcessor();
  }
}

/* System data refreshed at regular intervals */
void System::RefreshAttributes() {
  MemoryUtilization(LinuxParser::MemoryUtilization());
  UpTime(LinuxParser::UpTime());
  auto [totalProcs, runningProcs] = LinuxParser::FetchNumProcesses();
  TotalProcesses(totalProcs);
  RunningProcesses(runningProcs);
  RefreshProcesses();
  RefreshCpus();
}

/* Used to create a map of process ids with their corresponding objects */
std::map<int, Process*> System::MapPidToObj(vector<Process*>& processes) {
  std::map<int, Process*> idToObj;
  for (Process* p : processes) {
    idToObj[p->Pid()] = p;
  }
  return idToObj;
}
/* used for sorting purposes */
bool comparator(const Process* a, const Process* b) {
  return a->CpuUtilization() > b->CpuUtilization();
}

/* Builds out a new process container based on the pids */
/* If a new process has been created then new process object will be created
 * otherwise the existing process obj will be resued */
vector<Process*> System::BuildProcessContainer() {
  vector<Process*> processObjs;
  vector<int> activeProcessIds = LinuxParser::Pids();
  std::map<int, Process*> idToObj = MapPidToObj(Processes());
  for (int pid : activeProcessIds) {
    if (idToObj.find(pid) == idToObj.end())
      processObjs.emplace_back(new Process(pid));
    else
      processObjs.emplace_back(idToObj[pid]);
  }
  return processObjs;
}

void System::RefreshProcesses() {
  /* Process refreshing causes new cpu utilization value to be fetched */
  vector<Process*> processObjs = BuildProcessContainer();
  for (Process* p : processObjs) p->RefreshAttributes();
  std::sort(processObjs.begin(), processObjs.end(), comparator);
  processes_ = processObjs;
}

/* getters*/
int System::GetNumCpus() { return Cpus().size(); }
std::vector<Processor*>& System::Cpus() { return cpus_; }
vector<Process*>& System::Processes() { return processes_; }
std::string System::Kernel() const { return kernel_; }
float System::MemoryUtilization() const { return memortUtilization_; }
std::string System::OperatingSystem() const { return operatingSystem_; }
int System::RunningProcesses() const { return runningProcesses_; }
int System::TotalProcesses() const { return totalProcesses_; }
long System::UpTime() const { return upTime_; }

/* setters */
void System::OperatingSystem(std::string operatingSystem) {
  operatingSystem_ = operatingSystem;
}
void System::Kernel(std::string kernel) { kernel_ = kernel; }
void System::TotalProcesses(int totalProcs) { totalProcesses_ = totalProcs; };
void System::RunningProcesses(int runningProcs) {
  runningProcesses_ = runningProcs;
}
void System::UpTime(long upTime) { upTime_ = upTime; }
void System::MemoryUtilization(float memUtil) { memortUtilization_ = memUtil; }
void System::AddCpu(Processor* cpu) { cpus_.emplace_back(cpu); };