#include "system.h"

#include <unistd.h>

#include <algorithm>
#include <string>
#include <vector>

#include "linux_parser.h"
#include "process.h"
#include "processor.h"

using std::string;
using std::vector;

/* Initializing attribs that dont need to be updated with every refresh */
System::System() {
  /* Data for each cpu core is stored and maintained */
  for (int i = 0; i < LinuxParser::NumCores(); i++) {
    AddCpu(new Processor(i));
  }
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
  /* Kernel and OS name does not change with time*/
  if (Kernel().empty()) Kernel(LinuxParser::Kernel());
  if (OperatingSystem().empty())
    OperatingSystem(LinuxParser::OperatingSystem());
  /* Refreshed every second */
  RefreshProcesses();
  RefreshCpus();
  MemoryUtilization(LinuxParser::MemoryUtilization());
  UpTime(LinuxParser::UpTime());
  auto [totalProcs, runningProcs] = LinuxParser::FetchNumProcesses();
  TotalProcesses(totalProcs);
  RunningProcesses(runningProcs);
}

/* Used to create a map of process ids with their corresponding objects */
std::map<int, Process*> System::MapPidToObj(vector<Process*>& processes) {
  std::map<int, Process*> idToObj;
  for (Process* p : processes) {
    idToObj[p->Pid()] = p;
  }
  return idToObj;
}

/* Builds out a new process container based on the pids */
/* If a new process has been created then new process object will be created
 * otherwise the existing process obj will be resued */
vector<Process*> System::BuildProcessContainer() {
  vector<Process*> processObjs;
  vector<int> activeProcessIds = LinuxParser::Pids();
  std::map<int, Process*> idToObj = MapPidToObj(Processes());

  for (int pid : activeProcessIds) {
    Process* p =
        idToObj.find(pid) == idToObj.end() ? new Process(pid) : idToObj[pid];
    try {
      p->RefreshAttributes();
    } catch (std::exception& ex) {
      continue;
    }
    processObjs.emplace_back(p);
  }
  return processObjs;
}

/* Delete unused proc objects from heap to avoid memory leaks*/
void System::deleteOldProcObjs(vector<Process*>& oldObjs,
                               vector<Process*>& newObjs) {
  std::map<int, Process*> oldProcMap = MapPidToObj(oldObjs);
  std::map<int, Process*> newProcMap = MapPidToObj(newObjs);

  for (auto& [key, val] : oldProcMap) {
    if (newProcMap.find(key) == newProcMap.end()) delete val;
  }
}

void System::RefreshProcesses() {
  /* Process refreshing causes new cpu utilization value to be fetched */
  vector<Process*> processObjs = BuildProcessContainer();
  deleteOldProcObjs(processes_, processObjs);
  std::sort(processObjs.begin(), processObjs.end(),
            [](const Process* a, const Process* b) {
              return a->CpuUtilization() > b->CpuUtilization();
            });

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