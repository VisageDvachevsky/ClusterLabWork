struct Program {
    int arrival_time;
    int required_processors;
    int execution_time;
    int id; 
};

struct Cluster {
    int total_processors;
    int available_processors;
    std::vector<int> processors_usage; 
    int idle_time;
    double load_sum;
    int load_count;

    explicit Cluster(int processors) : 
        total_processors(processors), 
        available_processors(processors), 
        idle_time(0), 
        load_sum(0.0), 
        load_count(0) {
        processors_usage.resize(processors, 0);
    }

    void update_processors() {
        for (auto& time : processors_usage) {
            if (time > 0) time--;
        }
        available_processors = std::count(processors_usage.begin(), processors_usage.end(), 0);
    }

    bool allocate_program(const Program& program) {
        if (program.required_processors > available_processors) return false;

        int allocated = 0;
        for (auto& time : processors_usage) {
            if (time == 0) {
                time = program.execution_time;
                allocated++;
                if (allocated == program.required_processors) break;
            }
        }
        available_processors -= program.required_processors;
        return true;
    }

    void update_statistics() {
        int used_processors = total_processors - available_processors;
        load_sum += static_cast<double>(used_processors) / total_processors;
        load_count++;
        if (used_processors == 0) idle_time++;
    }

    double get_average_load() const {
        return load_count > 0 ? load_sum / load_count : 0.0;
    }
};