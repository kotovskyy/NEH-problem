from typing import List
import numpy as np
import math

class Task:
    def __init__(self, id: int, tpm: List[int]) -> None:
        self.id = id
        self.tpm = tpm
    
    def __repr__(self) -> str:
        return f"id: {self.id} | {[time for time in self.tpm]}"


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

def _getTasksTotals(data):
    totalTimes = []
    for i, task in enumerate(data):
        totalTimes.append([i, sum(task.tpm)])
    return sorted(totalTimes, key=lambda x: x[1], reverse=True)

def NEH(data):
    data = np.asarray(data)
    task_totals = _getTasksTotals(data)
    order = []
    for t in task_totals:
        C_max = math.inf
        index = None
        N_in_order = len(order)
        for i in range(N_in_order+1):
            order.insert(i, t[0])
            c = getTotalTime(data[order])
            if c < C_max:
                C_max = c
                index = i
            order.pop(i)
        order.insert(index, t[0])
    
    return order


def printOrder(order):
    print(" ".join([str(i+1) for i in order]))


def main():
    data = readData("data/data.txt")
    dataName = "data.000"
    data = data[dataName]
    order = NEH(data)
    print(f"DATASET : {dataName}")
    print(f"Min Cmax: {getTotalTime(np.asarray(data)[order])}")
    printOrder(order)

if __name__ == "__main__":
    main()
