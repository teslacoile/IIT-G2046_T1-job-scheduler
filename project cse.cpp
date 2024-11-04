#include <iostream>
#include <fstream>
#include <queue>
#include <vector>
#include <string>
#include <iomanip>

using namespace std;

const int WORKER_NODES = 128;
const int CORES_PER_NODE = 24;
const int MEMORY_PER_NODE = 64;

// Struct for Job
struct Job {
    int jobId;
    int arrivalTime;
    int coresRequired;
    int memoryRequired;
    int executionTime;  // in hours

    int grossValue() const {
        return executionTime * coresRequired * memoryRequired;
    }
};

// Comparator functions for different job queue policies
struct FCFSComparator {
    bool operator()(const Job &a, const Job &b) {
        return a.arrivalTime > b.arrivalTime;
    }
};

struct SmallestJobComparator {
    bool operator()(const Job &a, const Job &b) {
        return a.grossValue() > b.grossValue();
    }
};

struct ShortDurationComparator {
    bool operator()(const Job &a, const Job &b) {
        return a.executionTime > b.executionTime;
    }
};

// Struct for Worker Node
struct WorkerNode {
    int nodeId;
    int availableCores;
    int availableMemory;

    WorkerNode(int id) : nodeId(id), availableCores(CORES_PER_NODE), availableMemory(MEMORY_PER_NODE) {}
};

// Function to find a suitable worker node for a job based on selected allocation policy
WorkerNode* findWorkerNode(vector<WorkerNode> &nodes, const Job &job, const string &allocationPolicy) {
    WorkerNode* selectedNode = nullptr;

    if (allocationPolicy == "first_fit") {
        for (auto &node : nodes) {
            if (node.availableCores >= job.coresRequired && node.availableMemory >= job.memoryRequired) {
                selectedNode = &node;
                break;
            }
        }
    } else if (allocationPolicy == "best_fit") {
        for (auto &node : nodes) {
            if (node.availableCores >= job.coresRequired && node.availableMemory >= job.memoryRequired) {
                if (!selectedNode || (node.availableCores < selectedNode->availableCores &&
                                      node.availableMemory < selectedNode->availableMemory)) {
                    selectedNode = &node;
                }
            }
        }
    } else if (allocationPolicy == "worst_fit") {
        for (auto &node : nodes) {
            if (node.availableCores >= job.coresRequired && node.availableMemory >= job.memoryRequired) {
                if (!selectedNode || (node.availableCores > selectedNode->availableCores &&
                                      node.availableMemory > selectedNode->availableMemory)) {
                    selectedNode = &node;
                }
            }
        }
    }
    return selectedNode;
}

// Function to input jobs from user
vector<Job> inputJobsFromUser() {
    vector<Job> jobs;
    int numJobs;
    
    cout << "Enter the number of jobs: ";
    cin >> numJobs;

    for (int i = 0; i < numJobs; ++i) {
        Job job;
        job.jobId = i + 1;

        cout << "\nJob " << job.jobId << " Details:\n";
        cout << "Arrival Time (hours): ";
        cin >> job.arrivalTime;
        cout << "Cores Required: ";
        cin >> job.coresRequired;
        cout << "Memory Required (GB): ";
        cin >> job.memoryRequired;
        cout << "Execution Time (hours): ";
        cin >> job.executionTime;

        jobs.push_back(job);
    }
    return jobs;
}

// Simulation function
void simulateJobScheduling(vector<Job> &jobs, const string &queuePolicy, const string &allocationPolicy, ofstream &csvFile) {
    priority_queue<Job, vector<Job>, FCFSComparator> jobQueueFCFS;
    priority_queue<Job, vector<Job>, SmallestJobComparator> jobQueueSmallest;
    priority_queue<Job, vector<Job>, ShortDurationComparator> jobQueueShort;
    
    vector<WorkerNode> nodes;
    for (int i = 0; i < WORKER_NODES; ++i) {
        nodes.emplace_back(i);
    }

    // Fill the queue based on queue policy
    for (const auto &job : jobs) {
        if (queuePolicy == "fcfs") {
            jobQueueFCFS.push(job);
        } else if (queuePolicy == "smallest_job_first") {
            jobQueueSmallest.push(job);
        } else if (queuePolicy == "short_duration_first") {
            jobQueueShort.push(job);
        }
    }

    double totalCpuUsage = 0;
    double totalMemoryUsage = 0;
    int jobCount = 0;

    while (!jobQueueFCFS.empty() || !jobQueueSmallest.empty() || !jobQueueShort.empty()) {
        Job job;
        if (queuePolicy == "fcfs") {
            job = jobQueueFCFS.top();
            jobQueueFCFS.pop();
        } else if (queuePolicy == "smallest_job_first") {
            job = jobQueueSmallest.top();
            jobQueueSmallest.pop();
        } else if (queuePolicy == "short_duration_first") {
            job = jobQueueShort.top();
            jobQueueShort.pop();
        }

        WorkerNode *node = findWorkerNode(nodes, job, allocationPolicy);
        if (node) {
            node->availableCores -= job.coresRequired;
            node->availableMemory -= job.memoryRequired;

            totalCpuUsage += job.coresRequired;
            totalMemoryUsage += job.memoryRequired;
            jobCount++;
        }
    }

    double avgCpuUsage = totalCpuUsage / (WORKER_NODES * CORES_PER_NODE);
    double avgMemoryUsage = totalMemoryUsage / (WORKER_NODES * MEMORY_PER_NODE);

    csvFile << queuePolicy << "," << allocationPolicy << ","
            << fixed << setprecision(2) << avgCpuUsage * 100 << ","
            << fixed << setprecision(2) << avgMemoryUsage * 100 << endl;
}

int main() {
    vector<Job> jobs = inputJobsFromUser();

    string queuePolicy, allocationPolicy;
    ofstream csvFile("output.csv");

    if (!csvFile.is_open()) {
        cerr << "Error opening output file.\n";
        return 1;
    }
    
    csvFile << "QueuePolicy,AllocationPolicy,CPUUsage(%),MemoryUsage(%)\n";

    cout << "\nChoose Queue Policy (fcfs, smallest_job_first, short_duration_first): ";
    cin >> queuePolicy;
    cout << "Choose Allocation Policy (first_fit, best_fit, worst_fit): ";
    cin >> allocationPolicy;

    simulateJobScheduling(jobs, queuePolicy, allocationPolicy, csvFile);

    csvFile.close();
    cout << "Simulation complete. Results saved to output.csv\n";
    return 0;
}