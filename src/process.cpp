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
    // Retrieve jiffies data for the current and previous times
    vector<string> current_jiffies = LinuxParser::CpuUtilization();
    static vector<string> previous_jiffies = current_jiffies;

    // Initialize total jiffies and work jiffies for the current and previous times
    float current_total_jiffies = 0;
    float current_work_jiffies = 0;
    float previous_total_jiffies = 0;
    float previous_work_jiffies = 0;

    // Calculate current jiffies
    for (size_t i = 0; i < current_jiffies.size(); ++i) {
        float jiffies = stof(current_jiffies[i]);
        current_total_jiffies += jiffies;
        if (i < 3) { // User, Nice, System jiffies
            current_work_jiffies += jiffies;
        }
    }

    // Calculate previous jiffies
    for (size_t i = 0; i < previous_jiffies.size(); ++i) {
        float jiffies = stof(previous_jiffies[i]);
        previous_total_jiffies += jiffies;
        if (i < 3) {
            previous_work_jiffies += jiffies;
        }
    }

    // Calculate CPU utilization by the difference of current and previous jiffies
    float diff_total_jiffies = current_total_jiffies - previous_total_jiffies;
    float diff_work_jiffies = current_work_jiffies - previous_work_jiffies;

    // Save current jiffies as previous for the next round
    previous_jiffies = current_jiffies;

    // Avoid division by zero and calculate percentage utilization
    if (diff_total_jiffies == 0) return 0;
    return diff_work_jiffies / diff_total_jiffies;
}

// Return the command that generated this process
string Process::Command() { 
    return LinuxParser::Command(pid_);
}

// Return this process's memory utilization in MB
string Process::Ram() { 
    string ram = LinuxParser::Ram(pid_);
    float ram_usage = stof(ram);  // Convert string to float
    if (ram_usage > 0) {
        ram_usage /= 1024;  // Convert from KB to MB
    }
    return to_string(ram_usage).substr(0, 5);  // Limit to 2 decimal places
}

// Return the user (name) that generated this process
string Process::User() {
    string user_id = LinuxParser::Uid(pid_);
    return LinuxParser::User(user_id); 
}

// Return the age of this process (in seconds)
long int Process::UpTime() {
    string line;
    std::ifstream stream("/proc/" + std::to_string(pid_) + "/stat");
    if (stream.is_open()) {
        getline(stream, line);
        std::istringstream ss(line);
        string value;
        for (int i = 0; i < 21; i++) ss >> value;  // Skip first 21 values
        long int start_time;
        ss >> start_time;
        long int uptime = LinuxParser::UpTime() - start_time / sysconf(_SC_CLK_TCK);
        return uptime;
    }
    return 0;
}

// Overload the "less than" comparison operator for Process objects
bool Process::operator<(Process const& a[[maybe_unused]]) const { return true; }
