#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <string>
#include <algorithm>
#include <numeric>

class Task {
private:
    int id;
    std::vector<int> tpm;

public:
    Task(int id, const std::vector<int>& tpm) : id(id), tpm(tpm) {}

    const std::vector<int>& getTpm() const {
        return tpm;
    }

    int getId() const {
        return id;
    }

    void printTask() const {
        std::cout << "id: " << id << " | [";
        for (size_t i = 0; i < tpm.size(); ++i) {
            std::cout << tpm[i];
            if (i < tpm.size() - 1) {
                std::cout << ", ";
            }
        }
        std::cout << "]" << std::endl;
    }
};

std::vector<std::vector<Task>> readData(const std::string& filepath) {
    // opening the file
    std::ifstream file(filepath);
    std::vector<std::vector<Task>> datasets;
    // to store lines read from the file
    std::string line;

    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filepath << std::endl;
        return datasets;
    }

    // vector to temporarily store tasks of the current dataset
    std::vector<Task> current_dataset;
    bool save_data = false;
    int counter = 0;

    // loop until the end of the file
    while (std::getline(file, line)) {
        if (line.empty()){
            save_data = false;
            if (!current_dataset.empty()) {
                datasets.push_back(current_dataset);
                current_dataset.clear();
            }
            continue;
        }
        if (line.find("data.") != std::string::npos){
            save_data = true;
            counter = 0;
        } else {
            if (save_data){
                if (counter == 0){
                    counter++;
                    continue;
                } 
                std::istringstream iss(line);
                int num;
                std::vector<int> task_tpm;
                while (iss >> num) {
                    task_tpm.push_back(num);
                }
                current_dataset.emplace_back(counter, task_tpm);
                counter++;
            }
        }

    }
    // Add the last dataset if it's not empty
    if (!current_dataset.empty()) {
        datasets.push_back(current_dataset);
    }

    file.close();
    return datasets;
}

std::vector<int> getTaskInputOrder(const std::vector<Task>& data) {
  // Sort the data in descending order based on the sum of tpm elements
  std::vector<Task> sorted_data = data;
  std::sort(sorted_data.begin(), sorted_data.end(), [](const Task& a, const Task& b) {
    return std::accumulate(a.getTpm().begin(), a.getTpm().end(), 0) > 
           std::accumulate(b.getTpm().begin(), b.getTpm().end(), 0);
  });

  std::vector<int> order;

  std::transform(sorted_data.begin(), sorted_data.end(), std::back_inserter(order), [](const Task& t) {
    return t.getId() - 1;
  });

  return order;
}

int main() {
    std::string filepath = "data/data.txt";
    std::vector<std::vector<Task>> datasets = readData(filepath);
    int dataset_index = 1;
    std::vector<Task> data = datasets[dataset_index];

    std::vector<int> order = getTaskInputOrder(data);


    return 0;
}