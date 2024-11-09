#include <unistd.h>
#include <cctype>
#include <sstream>
#include <string>
#include <vector>

#include "process.h"
#include "linux_parser.h"

using std::string;
using std::to_string;
using std::vector;

// Return this process's ID
int Process::Pid() { return pid_; }

// Return this process's CPU utilization
float Process::CpuUtilization() const {
    // Retrieve the jiffies data for calculating CPU utilization
    vector<string> jiffies_data = LinuxParser::CpuUtilization();
    float total_jiffies = 0;
    float work_jiffies = 0;

    // Sum up the jiffies for total CPU time and work time (User, Nice, System jiffies)
    for (size_t i = 0; i < jiffies_data.size(); i++) {
        float jiffies = stof(jiffies_data[i]);
        total_jiffies += jiffies;
        if (i < 3) {  // User, Nice, System jiffies
            work_jiffies += jiffies;
        }
    }

    // Calculate CPU utilization as a fraction between 0 and 1
    float cpu_percentage = work_jiffies / total_jiffies; // Remove * 100 here
    return cpu_percentage;
}

// Return the command that generated this process
string Process::Command() { 
    return LinuxParser::Command(pid_);
}

// Return this process's memory utilization
string Process::Ram() { return LinuxParser::Ram(pid_); }

// Return the user (name) that generated this process
string Process::User() {
    string user_id = LinuxParser::Uid(pid_);
    return LinuxParser::User(user_id); 
}

// Return the age of this process (in seconds)
long int Process::UpTime() {
    string line;
    string value;
    long int uptime = 0;

    std::ifstream stream("/proc/" + std::to_string(pid_) + "/stat");
    if (stream.is_open()) {
        std::getline(stream, line);
        std::istringstream linestream(line);
        // Skip the first 13 fields (pid, comm, state, etc.)
        for (int i = 0; i < 13; ++i) {
            linestream >> value;
        }
        long int starttime;
        linestream >> starttime;  // Time when the process started
        uptime = LinuxParser::UpTime() - (starttime / sysconf(_SC_CLK_TCK));
    }

    return uptime; // Return process uptime in seconds
}

// Overload the "less than" comparison operator for Process objects
bool Process::operator<(Process const& a[[maybe_unused]]) const { return true; }
