#include <dirent.h>
#include <unistd.h>
#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include "iostream"
#include "linux_parser.h"

using std::stof;
using std::string;
using std::to_string;
using std::vector;

// This function reads the operating system's name from a specific file.
string LinuxParser::OperatingSystem() {
  string line, key, value;
  std::ifstream file_stream(kOSPath);
  if (file_stream.is_open()) {
    while (std::getline(file_stream, line)) {
      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), '=', ' ');
      std::replace(line.begin(), line.end(), '"', ' ');
      std::istringstream line_stream(line);
      while (line_stream >> key >> value) {
        if (key == "PRETTY_NAME") {
          std::replace(value.begin(), value.end(), '_', ' ');
          return value;
        }
      }
    }
  }
  return value;
}

// Reads and returns the kernel version from the system file.
string LinuxParser::Kernel() {
  string os, version, kernel;
  string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream line_stream(line);
    line_stream >> os >> version >> kernel;
  }
  return kernel;
}

// Reads and returns a list of PIDs by iterating over the directory contents.
vector<int> LinuxParser::Pids() {
  vector<int> pids;
  DIR* directory = opendir(kProcDirectory.c_str());
  struct dirent* file;
  while ((file = readdir(directory)) != nullptr) {
    string filename(file->d_name);
    if (file->d_type == DT_DIR && std::all_of(filename.begin(), filename.end(), isdigit)) {
      int pid = stoi(filename);
      pids.push_back(pid);
    }
  }
  closedir(directory);
  return pids;
}

// This function calculates memory utilization by reading memory info from the file.
float LinuxParser::MemoryUtilization() {
  float memTotal, memFree;
  string line;
  std::ifstream stream(kProcDirectory + kMeminfoFilename);

  if (stream.is_open()) {
    int lineCount = 0;
    vector<float> values;
    while (lineCount < 10) {
      std::getline(stream, line);
      std::istringstream line_stream(line);
      string title, value;
      line_stream >> title >> value;
      values.push_back(std::stof(value));
      lineCount++;
    }
    memTotal = values[0];
    memFree = values[1];
    float cached = values[4];
    float memAvailable = values[2];
    return (cached + memAvailable) / memTotal;
  }
  return 0.0;
}

// Reads and returns system uptime.
long LinuxParser::UpTime() {
  long uptime;
  string line;
  std::ifstream stream(kProcDirectory + kUptimeFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream line_stream(line);
    line_stream >> uptime;
  }
  return uptime;
}

// Calculates total jiffies by summing up jiffies read from the system file.
long LinuxParser::Jiffies() {
  vector<string> result = GetJiffies();
  long total = 0;
  for (const auto& value : result) {
    total += stol(value);
  }
  return total;
}

// Reads and returns active jiffies for the entire system.
long LinuxParser::ActiveJiffies() {
  vector<string> result = GetJiffies();
  long active_sum = 0;
  for (int i = 0; i < 3; i++) {
    active_sum += stol(result[i]);
  }
  return active_sum;
}

// Reads and returns idle jiffies for the entire system.
long LinuxParser::IdleJiffies() {
  vector<string> result = GetJiffies();
  long idle_sum = 0;
  for (size_t i = 3; i < result.size(); i++) {
    idle_sum += stol(result[i]);
  }
  return idle_sum;
}

// This function reads and returns CPU utilization as a vector of jiffies.
vector<string> LinuxParser::CpuUtilization() {
  return GetJiffies();
}

// Retrieves the total number of processes recorded by the system.
int LinuxParser::TotalProcesses() {
  return std::stoi(FindValueByKey(kProcDirectory + kStatFilename, "processes"));
}

// Retrieves the count of running processes in the system.
int LinuxParser::RunningProcesses() {
  return std::stoi(FindValueByKey(kProcDirectory + kStatFilename, "procs_running"));
}

// Fetches the command associated with a specific process by its PID.
string LinuxParser::Command(int pid) {
  string file_path = kProcDirectory + to_string(pid) + kCmdlineFilename;
  string line;
  std::ifstream file_stream(file_path);
  if (file_stream.is_open()) {
    std::getline(file_stream, line);
  }
  return line;
}

// Returns the memory usage for a specific process in MB or KB format.
string LinuxParser::Ram(int pid) {
  string file_path = kProcDirectory + to_string(pid) + kStatusFilename;
  string ram_value = FindValueByKey(file_path, "VmSize");
  return (stof(ram_value) >= 1024) ? to_string(stof(ram_value) / 1024) + " MB" : ram_value + " KB";
}

// Retrieves the user ID associated with a specific process.
string LinuxParser::Uid(int pid) {
  string file_path = kProcDirectory + to_string(pid) + kStatusFilename;
  return FindValueByKey(file_path, "Uid");
}

// Finds the username corresponding to a given UID.
std::string LinuxParser::User(std::string uid) {
  string line, name, x, uid_key;
  std::ifstream file_stream(kPasswordPath);
  if (file_stream.is_open()) {
    while (std::getline(file_stream, line)) {
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream line_stream(line);
      line_stream >> name >> x >> uid_key;
      if (uid_key == uid) {
        return name;
      }
    }
  }
  return name;
}

// This function finds a specific key in a file and returns its value.
std::string LinuxParser::FindValueByKey(std::string file_path, std::string key) {
  string line, line_key, value;
  std::ifstream file_stream(file_path);
  if (file_stream.is_open()) {
    while (std::getline(file_stream, line)) {
      std::istringstream line_stream(line);
      line_stream >> line_key >> value;
      if (line_key == key) {
        return value;
      }
    }
  }
  return value;
}

// Reads CPU jiffies data and returns it as a vector of strings.
std::vector<string> LinuxParser::GetJiffies() {
  vector<string> jiffies_data;
  string line, jiffy;
  std::ifstream file_stream(kProcDirectory + kStatFilename);
  if (file_stream.is_open()) {
    std::getline(file_stream, line);
    std::istringstream line_stream(line);
    int field_count = 0;
    while (line_stream >> jiffy) {
      if (++field_count > 2) {
        jiffies_data.push_back(jiffy);
      }
    }
  }
  return jiffies_data;
}
