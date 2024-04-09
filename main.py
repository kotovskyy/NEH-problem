from typing import List

def readData(filepath: str) -> dict[str: List[str]]:
    """
    Reads data from a file with sections defined by "data.XXX" lines.

    Args:
        - `filepath: str` - Path to the file containing the data.

    Returns:
        - `dict` - A dictionary where keys are section names ("data.XXX") and values are lists of lines within that section.
    """
    data = {}
    current_section = None
    with open(filepath, 'r') as f:
        saveData = False
        for line in f:
            line = line.strip()
            if not line:
                saveData = False
                continue    

            if line.startswith("data."):
                saveData = True
                current_section = line[:-1]
                data[current_section] = []
            else:
                if current_section and saveData:
                    data[current_section].append(line)
    return data

def main():
    data = readData("data/data.txt")
    data = data["data.000"]
    print(data)

if __name__ == "__main__":
    main()
