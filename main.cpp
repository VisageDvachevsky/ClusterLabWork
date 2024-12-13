#include <iostream>
#include <queue>
#include <vector>
#include <random>
#include <algorithm>
#include <iomanip>
#include <optional>
#include <stdexcept>

struct Task {
    int id;
    int arrival_time;
    int required_processors;
    int execution_time;

    bool operator<(const Task& other) const {
        return required_processors > other.required_processors;
    }
};

struct ClusterStatistics {
    int total_tasks = 0;
    int completed_tasks = 0;
    int idle_ticks = 0;
    double load_accumulated = 0;
    int load_measurements = 0;

    void display(int total_processors) const {
        std::cout << "Results:\n";
        std::cout << "Total processors: " << total_processors << "\n";
        std::cout << "Total tasks generated: " << total_tasks << "\n";
        std::cout << "Total tasks completed: " << completed_tasks << "\n";
        std::cout << "Total idle ticks: " << idle_ticks << "\n";
        std::cout << "Average cluster load: " << std::fixed << std::setprecision(2)
                  << (load_measurements > 0 ? load_accumulated / load_measurements * 100 : 0.0)
                  << "%\n";
    }
};

class Cluster {
public:
    explicit Cluster(int processor_count)
        : total_processors_(processor_count), available_processors_(processor_count),
          processor_status_(processor_count, 0) {}

    void update_processors() {
        for (auto& time : processor_status_) {
            if (time > 0) {
                --time;  
            }
        }
        available_processors_ = std::count(processor_status_.begin(), processor_status_.end(), 0);
    }

    bool allocate_task(const Task& task) {
        if (task.required_processors > available_processors_) {
            return false;  
        }

        int allocated = 0;
        for (auto& time : processor_status_) {
            if (time == 0) {  
                time = task.execution_time;
                ++allocated;
                if (allocated == task.required_processors) {
                    break;
                }
            }
        }
        available_processors_ -= task.required_processors;
        return true;
    }

    void record_statistics(ClusterStatistics& stats) const {
        int used_processors = total_processors_ - available_processors_;
        stats.load_accumulated += static_cast<double>(used_processors) / total_processors_;
        ++stats.load_measurements;
        if (used_processors == 0) {
            ++stats.idle_ticks;  
        }
    }

    int get_total_processors() const {
        return total_processors_;
    }

private:
    int total_processors_;
    int available_processors_;
    std::vector<int> processor_status_;  
};

class TaskManager {
public:
    TaskManager(double task_spawn_probability, int min_processors, int max_processors, int min_exec_time, int max_exec_time)
        : task_spawn_probability_(task_spawn_probability),
          processors_distribution_(min_processors, max_processors),
          execution_time_distribution_(min_exec_time, max_exec_time),
          task_id_counter_(1) {
        if (task_spawn_probability_ < 0.0 || task_spawn_probability_ > 1.0) {
            throw std::invalid_argument("Task spawn probability must be between (0;1) КТО ПРОЧИТАЛ ГЕЙ");
        }
    }

    std::optional<Task> maybe_generate_task(int current_time) {
        if (task_spawn_distribution_(random_generator_)) {
            return Task{
                .id = task_id_counter_++,
                .arrival_time = current_time,
                .required_processors = processors_distribution_(random_generator_),
                .execution_time = execution_time_distribution_(random_generator_)
            };
        }
        return std::nullopt;  
    }

private:
    double task_spawn_probability_;
    std::uniform_int_distribution<> processors_distribution_;
    std::uniform_int_distribution<> execution_time_distribution_;
    int task_id_counter_;
    std::mt19937 random_generator_{std::random_device{}()};
    std::bernoulli_distribution task_spawn_distribution_{task_spawn_probability_};
};

class ClusterSimulation {
public:
    ClusterSimulation(int processors, double spawn_probability, int simulation_ticks,
                      int min_processors, int max_processors, int min_exec_time, int max_exec_time)
        : cluster_(processors),
          task_manager_(spawn_probability, min_processors, max_processors, min_exec_time, max_exec_time),
          max_ticks_(simulation_ticks) {}

    void run() {
        for (int tick = 0; tick < max_ticks_; ++tick) {
            cluster_.update_processors();

            if (auto task = task_manager_.maybe_generate_task(tick)) {
                task_queue_.push(*task);
                stats_.total_tasks++;
            }

            process_task_queue();

            cluster_.record_statistics(stats_);
        }

        stats_.display(cluster_.get_total_processors());
    }

private:
    Cluster cluster_;
    TaskManager task_manager_;
    std::priority_queue<Task> task_queue_;
    ClusterStatistics stats_;
    int max_ticks_;

    void process_task_queue() {
        while (!task_queue_.empty()) {
            Task task = task_queue_.top();
            task_queue_.pop();

            if (!cluster_.allocate_task(task)) {
                task_queue_.push(task);
            } else {
                stats_.completed_tasks++;
            }
        }
    }
};

int main() {
    constexpr int kMinProcessors = 16;
    constexpr int kMaxProcessors = 64;
    constexpr int kSimulationTicks = 1000;
    constexpr double kTaskSpawnProbability = 0.3;
    constexpr int kMinExecTime = 1, kMaxExecTime = 10;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> processor_distribution(kMinProcessors, kMaxProcessors);
    int processors = processor_distribution(gen);

    ClusterSimulation simulation(processors, kTaskSpawnProbability, kSimulationTicks,
                                 1, 8, kMinExecTime, kMaxExecTime);
    simulation.run();

    return 0;
}
