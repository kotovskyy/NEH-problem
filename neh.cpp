#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <string>
#include <algorithm>
#include <numeric>

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
        Task task = data[order[i]];
        for (int m = 0; m < M; m++) {
            t = std::max(t, prev[m]) + task.getTpm()[m];
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
    // if (k == -1) {
    //     k = order.size()-1;
    // }
    int M = data[0].getTpm().size();
    std::vector<int> prev(M, 0);
    if (k != order.size()-1){
        prev = getMatrixColumn(backward, order[k+1]);
    }
    for (int i = k; i >= 0; i--) {
        int t = 0;
        Task task = data[order[i]];
        for (int m = M-1; m >= 0; m--) {
            t = std::max(t, prev[m]) + task.getTpm()[m];
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
    std::vector<int> prev(M, 0);
    if (k != 0){
        prev = getMatrixColumn(forward, order[k-1]);
    }
    for (int m = 0; m < M; m++) {
        t = std::max(t, prev[m]) + task.getTpm()[m];
        task_table[m] = t;
        prev[m] = t;
    }
    if (k == N){
        return t;
    } 
    int cmax = 0;
    for (int i = 0; i < M; i++){
        int value = task_table[i] + backward[i][order[k]];
        if (value > cmax){
            cmax = value;
        }
    }
    return cmax; 
}


std::vector<int> QNEH(const std::vector<Task>& data) {
    std::vector<int> input_order = getTaskInputOrder(data);
    for (int i : input_order) {
        std::cout << i << " ";
    }
    std::cout << std::endl;
    std::vector<int> order;
    int index = 0;
    int N_tasks = data.size();
    int M = data[0].getTpm().size();
    std::vector<std::vector<int>> forward(M, std::vector<int>(N_tasks, 0));
    std::vector<std::vector<int>> backward(M, std::vector<int>(N_tasks, 0)); 

    for (int t_index : input_order) {
        int N = order.size();
        int C_max = 9999999; // big number instead of infinity
        std::cout << t_index << std::endl;
        updateForwardTable(data, order, forward, index);
        printMatrix(forward);
        updateBackwardTable(data, order, backward, index);
        printMatrix(backward);
        for (int i = 0; i < N+1; i++){
            int c = getQNEHCmax(data[t_index], forward, backward, i, order);
            if (c < C_max) {
                C_max = c;
                index = i;
            }
        }
        order.insert(order.begin() + index, t_index);
    }

    updateForwardTable(data, order, forward, index);
    updateBackwardTable(data, order, backward, index);
    printMatrix(forward);
    printMatrix(backward);


    return order;
}

int main() {
    std::string filepath = "data/data.txt";
    std::vector<std::vector<Task>> datasets = readData(filepath);
    int dataset_index = 0;
    // std::vector<Task> data = datasets[dataset_index];
    // std::vector<int> result = QNEH(data);
    // for (int i : result) {
    //     std::cout << i << " ";
    // }
    // std::vector<std::vector<int>> matrix = {{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};
    // std::vector<int> column = getMatrixColumn(matrix, 2);

    // std::vector<Task> data = {Task(1, {1, 1, 3}),
    //                           Task(2, {3, 4, 3}), 
    //                           Task(3, {4, 1, 2}),
    //                           Task(4, {2, 4, 1}),
    //                           Task(5, {1, 2, 3})};
    
    std::vector<Task> data = {Task(1, {1, 1, 3}),
                              Task(2, {3, 4, 3}), 
                              Task(3, {4, 1, 2}),
                              Task(4, {2, 4, 1})};
    

    std::vector<int> result = QNEH(data);

    for (int i : result) {
        std::cout << i << " ";
    }
    
    // std::vector<std::vector<int>> forward(3, std::vector<int>(5, 0));
    // updateForwardTable(data, {0, 1, 2, 3}, forward);
    // printMatrix(forward);
    // updateForwardTable(data, {4, 0, 1, 2, 3}, forward, 0);
    // printMatrix(forward);

    // std::vector<std::vector<int>> backward(3, std::vector<int>(5, 0));
    // updateBackwardTable(data, {0, 1, 2, 3}, backward);
    // printMatrix(backward);
    // updateBackwardTable(data, {0, 1, 4, 2, 3}, backward, 2);
    // printMatrix(backward);

    // printMatrix(forward);
    // printMatrix(backward);

    // updateForwardTable(data, {}, forward, 0);
    // printMatrix(forward);
    
    // updateBackwardTable(data, {}, backward, 0);

    // printMatrix(backward);

    

    // for (int i = 0; i < 3; ++i) {
    //     for (int j = 0; j < 3; ++j) {
    //         std::cout << matrix[i][j] << " ";
    //     }
    //     std::cout << std::endl;
    // }

    // for (int i : column) {
    //     std::cout << i << " ";
    // }

    return 0;
}