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
    N_machines = len(data[0].tpm)
    Cmax = 0
    machine_free_at = [0] * N_machines
    for task in data:
        t = 0
        for m in range(N_machines):
            task_time = task.tpm[m]
            Cmax = max(machine_free_at[m], t) + task_time
            t = Cmax
            machine_free_at[m] = Cmax
    return Cmax

def _createCmaxTable(data, N, M, backward=False):
    prev = np.zeros(M)
    output = np.zeros((M, N))
    if backward:
        for i in range(N-1, -1, -1):
            t = 0
            for m in range(M-1, -1, -1):
                task_time = data[i].tpm[m]
                Cmax = max(prev[m], t) + task_time
                t = Cmax
                prev[m] = Cmax
                output[m, i] = Cmax
    else:
        for i, task in enumerate(data):
            t = 0
            for m in range(M):
                task_time = task.tpm[m]
                Cmax = max(prev[m], t) + task_time
                t = Cmax
                prev[m] = Cmax
                output[m, i] = Cmax
    return output

def QNEHTotalTime(task, forward, backward, N, M, i):
    t = 0
    task_table = np.zeros((M, 1))
    for m in range(M):
        task_time = task.tpm[m]
        Cmax = max(forward[m, i-1], t) + task_time if i > 0 else t + task_time
        t = Cmax
        task_table[m, 0] = Cmax
    result = np.hstack((forward[:, :i], task_table, backward[:, i:]))
    
    return max(result[:, i] + result[:, i+1]) if i < N else max(result[:, i])


def _getTasksTotals(data):
    return sorted([[i, sum(task.tpm)] for i, task in enumerate(data)], key=lambda x: x[1], reverse=True)


@calculate_time
def QNEH(data):
    data = np.asarray(data)
    task_totals = _getTasksTotals(data)
    order, index = [], None
    N, M = len(order), len(data[0].tpm)
    
    for t in task_totals:
        C_max = math.inf
        forward = _createCmaxTable(data[order], N, M, False)
        backward = _createCmaxTable(data[order], N, M, True)
        for i in range(N+1):
            c = QNEHTotalTime(data[t[0]], forward, backward, N, M, i)
            if c < C_max:
                C_max, index = c, i
        order.insert(index, t[0])
    return order
    
@calculate_time
def NEH(data):
    data = np.asarray(data)
    task_totals = _getTasksTotals(data)
    order, index = [], None
    
    for t in task_totals:
        C_max = math.inf
        for i in range(len(order)+1):
            new_order = order[:i] + [t[0]] + order[i:]
            c = getTotalTime(data[new_order])
            if c < C_max:
                C_max, index = c, i
        order.insert(index, t[0])
    return order


def printOrder(order):
    print("Order: " + " ".join([str(i+1) for i in order]))


def testSolution(data, datasetName: str, func) -> None:
    data = data[datasetName]
    print(f"DATASET : {datasetName}")
    order = func(data)
    print(f"Min Cmax: {getTotalTime(np.asarray(data)[order])}")
    printOrder(order)


def main():
    dataName = "data.120"
    data = readData("data/data.txt")
    testSolution(data, dataName, QNEH)
 
    
if __name__ == "__main__":
    main()
