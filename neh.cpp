#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <string>
#include <algorithm>
#include <numeric>
#include <chrono>
#include <limits>

void printMatrix(const std::vector<std::vector<int>>& matrix) {
    for (const std::vector<int>& row : matrix) {
        for (const int i : row) {
            std::cout << i << " ";
        }
        std::cout << std::endl;
    }
}

class Task {
private:
    int id;
    std::vector<int> tpm;
    int tpm_sum;

public:
    Task(int id, const std::vector<int>& tpm, int tpm_sum) : id(id), tpm(tpm), tpm_sum(tpm_sum){}

    const std::vector<int>& getTpm() const {
        return tpm;
    }

    int getId() const {
        return id;
    }

    int getTpmSum() const {
        return tpm_sum;
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
                int tpm_sum = 0;
                std::vector<int> task_tpm;
                while (iss >> num) {
                    tpm_sum += num;
                    task_tpm.push_back(num);
                }
                current_dataset.emplace_back(counter, task_tpm, tpm_sum);
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
  std::stable_sort(sorted_data.begin(), sorted_data.end(), [](const Task& a, const Task& b) {
    return a.getTpmSum() > b.getTpmSum();
  });

  std::vector<int> order;

  std::transform(sorted_data.begin(), sorted_data.end(), std::back_inserter(order), [](const Task& t) {
    return t.getId() - 1;
  });

  return order;
}

std::vector<int> getMatrixColumn(const std::vector<std::vector<int>>& matrix, int col){
    std::vector<int> column;
    int M = matrix.size();
    for (int i = 0; i < M; ++i){
        column.push_back(matrix[i][col]);
    }
    return column;
}

void updateForwardTable(const std::vector<Task>& data,
                        const std::vector<int>& order,
                        std::vector<std::vector<int>>& forward,
                        int k=0){
    int M = data[0].getTpm().size();
    std::vector<int> prev(M, 0);
    if (k != 0){
        prev = getMatrixColumn(forward, order[k-1]);
    }
    for (int i = k; i < order.size(); i++) {
        int t = 0;
        for (int m = 0; m < M; m++) {
            t = std::max(t, prev[m]) + data[order[i]].getTpm()[m];
            forward[m][order[i]] = t;
            prev[m] = t;
        }
    }
}


void updateBackwardTable(const std::vector<Task>& data,
                         const std::vector<int>& order,
                         std::vector<std::vector<int>>& backward,
                         int k=-1){
    if (order.size() == 0) {
        return;
    }
    int M = data[0].getTpm().size();
    std::vector<int> prev(M, 0);
    if (k != order.size()-1){
        prev = getMatrixColumn(backward, order[k+1]);
    }
    for (int i = k; i >= 0; i--) {
        int t = 0;
        for (int m = M-1; m >= 0; m--) {
            t = std::max(t, prev[m]) + data[order[i]].getTpm()[m];
            backward[m][order[i]] = t;
            prev[m] = t;
        }
    }
}


int getQNEHCmax(const Task& task,
                const std::vector<std::vector<int>>& forward,
                const std::vector<std::vector<int>>& backward,
                int k,
                const std::vector<int>& order){
    int t = 0;
    int N = order.size();
    int M = task.getTpm().size();
    std::vector<int> task_table(M, 0);
    std::vector<int> prev;
    int cmax = 0;
    if (k != 0){
        prev = getMatrixColumn(forward, order[k-1]);
    } else {
        prev = std::vector<int>(M, 0);
    }
    for (int m = 0; m < M; m++) {
        t = std::max(t, prev[m]) + task.getTpm()[m];
        task_table[m] = t;
        if (k < N) {
            int value = task_table[m] + backward[m][order[k]];
            if (value > cmax){
                cmax = value;
            }
        }
    }
    if (k == N){
        return t;
    } 
    return cmax; 
}


std::vector<int> QNEH(const std::vector<Task>& data) {
    std::vector<int> input_order = getTaskInputOrder(data);
    std::vector<int> order;
    int N_tasks = data.size();
    int M = data[0].getTpm().size();
    int index = 0;
    std::vector<std::vector<int>> forward(M, std::vector<int>(N_tasks, 0));
    std::vector<std::vector<int>> backward(M, std::vector<int>(N_tasks, 0)); 

    for (int t_index : input_order) {
        int N = order.size();
        int C_max = std::numeric_limits<int>::max();
        updateForwardTable(data, order, forward, index);
        updateBackwardTable(data, order, backward, index);
        for (int i = 0; i < N+1; i++){
            int c = getQNEHCmax(data[t_index], forward, backward, i, order);
            if (c < C_max) {
                C_max = c;
                index = i;
            }
        }
        order.insert(order.begin() + index, t_index);
    }
    return order;
}

int getTotalTime(const std::vector<Task>& data, const std::vector<int>& order){
    int N = data.size();
    int M = data[0].getTpm().size();
    int cmax = 0;
    std::vector<int> prev(M, 0);
    for (int t_index : order) {
        int t = 0;
        for (int m = 0; m < M; m++) {
            t = std::max(t, prev[m]) + data[t_index].getTpm()[m];
            prev[m] = t;
            cmax = t;
        }
    }
    return cmax;
}

void testSingle(int data_index, const std::vector<Task>& data, std::chrono::duration<double>& total_time) {
    printf("data.%03d : ", data_index);
    auto start = std::chrono::high_resolution_clock::now();
    std::vector<int> result = QNEH(data);
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end-start;
    
    total_time += duration;
    std::cout << getTotalTime(data, result) << " ";
    std::cout << duration.count() << " s"<< std::endl;
}

void testMultiple(int data_index_from, int data_index_to, const std::vector<std::vector<Task>>& datasets) {
    std::chrono::duration<double> total_time = std::chrono::duration<double>::zero();
    for (int i = data_index_from; i <= data_index_to; i++) {
        testSingle(i, datasets[i], total_time);
    }
    std::cout << "Total time: " << total_time.count() << " s"<< std::endl;
}

int main() {
    std::string filepath = "data/data.txt";
    std::vector<std::vector<Task>> datasets = readData(filepath);
    testMultiple(110, 120, datasets);
    return 0;
}
