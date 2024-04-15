from typing import List
import numpy as np
import math


class Task:
    def __init__(self, id: int, tpm: List[int]) -> None:
        self.id = id
        self.tpm = tpm
    
    def __repr__(self) -> str:
        return f"id: {self.id} | {[time for time in self.tpm]}"


def calculate_time(func):
    """
        Decorator to calculate total execution time of a function.
    """
    def inner(*args, **kwargs):
        import time
        start = time.time()
        order = func(*args, **kwargs)
        end = time.time()
        totalTime = end - start
        print(f"Execution time: {totalTime:.3} s")
        return order
        
    return inner


def readData(filepath: str) -> dict[str: List[Task]]:
    """
    Reads data from a file with sections defined by "data.XXX" lines.

    Args:
        - `filepath: str` - Path to the file containing the data.

    Returns:
        - `dict` - A dictionary where keys are section names ("data.XXX") and values are lists of lines within that section.
    """
    data = {}
    current_section = None
    counter = 0
    with open(filepath, 'r') as f:
        saveData = False
        for line in f:
            line = line.strip()
            if not line:
                saveData = False
                continue    

            if line.startswith("data."):
                saveData = True
                counter = 0
                current_section = line[:-1]
                data[current_section] = []
            else:
                if current_section and saveData:
                    if counter == 0:
                        counter += 1    
                        continue
                    newTask = Task(counter, [int(item) for item in line.split(" ")])
                    data[current_section].append(newTask)
                    counter += 1 
    return data


def getTotalTime(data):
    M = len(data[0].tpm)
    machine_time = np.zeros(M)
    Cmax = 0
    for task in data:
        task_frees_at = 0
        for m in range(M):
            entry_time = max(machine_time[m], task_frees_at)
            exit_time = entry_time + task.tpm[m]
            machine_time[m] = exit_time
            Cmax = task_frees_at = exit_time
    return int(Cmax)


def _createCmaxTable(data, N, M):
    forward = np.zeros((M, N))
    backward = np.zeros((M, N))

    prev = np.zeros(M)
    for i in range(N):
        t = 0
        task_times = data[i].tpm
        for m in range(M):
            task_time = task_times[m]
            t = max(prev[m], t) + task_time
            prev[m] = forward[m, i] = t

    prev = np.zeros(M)
    for i in range(N-1, -1, -1):
        t = 0
        task_times = data[i].tpm
        for m in range(M-1, -1, -1):
            task_time = task_times[m]
            t = max(prev[m], t) + task_time
            prev[m] = backward[m, i] = t

    return forward, backward


def QNEHTotalTime(task, forward, backward, N, M, i):
    t = 0
    task_table = np.zeros((M, 1))
    machine_time = forward[:, i-1] if i > 0 else np.zeros(M)
    for m in range(M):
        task_time = task.tpm[m]
        t = max(machine_time[m], t) + task_time
        task_table[m, 0] = t
        
    if i < N:
        return np.max(task_table[:, 0] + backward[:, i])
    return t


def _getTaskInputOrder(data):
    return [t.id-1 for t in sorted(data, key=lambda x: np.sum(x.tpm), reverse=True)]


@calculate_time
def QNEH(data):
    data = np.asarray(data)
    input_order = _getTaskInputOrder(data)
    order, index = [], None
    M = len(data[0].tpm)
    for t_index in input_order:
        N = len(order) 
        C_max = math.inf
        forward, backward = _createCmaxTable(data[order], N, M)
        for i in range(N+1):
            c = QNEHTotalTime(data[t_index], forward, backward, N, M, i)
            if c < C_max:
                C_max, index = c, i
        order.insert(index, t_index)
    return order
    
@calculate_time
def NEH(data):
    task_totals = _getTaskInputOrder(data)
    order, index = [], None
    
    for t_index in task_totals:
        C_max = math.inf
        for i in range(len(order)+1):
            new_order = order[:i] + [t_index] + order[i:]
            c = getTotalTime(data[new_order])
            if c < C_max:
                C_max, index = c, i
        order.insert(index, t_index)
    return order


def printOrder(order):
    print("Order: " + " ".join([str(i+1) for i in order]))


def testSolution(data, datasetName: str, func) -> None:
    data = np.asarray(data[datasetName])
    print(f"DATASET : {datasetName}")
    order = func(data)
    print(f"Min Cmax: {getTotalTime(data[order])}")
    printOrder(order)


def main():
    dataName = "data.120"
    data = readData("data/data.txt")
    testSolution(data, dataName, QNEH)
    # data = np.array([Task(1, [1, 1, 3]),
    #                  Task(2, [3, 4, 3]),
    #                  Task(3, [4, 1, 2]),
    #                  Task(4, [2, 4, 1])])
    # print(_createCmaxTable(data, 4, 3))
    # order = QNEH(data)
    # print(f"Total time: {getTotalTime(np.asarray(data)[order])}")
    # printOrder(order)
    
if __name__ == "__main__":
    main()
