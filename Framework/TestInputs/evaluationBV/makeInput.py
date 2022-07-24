import numpy as np

for inserts in range(10, 26, 1):
    numberOfInserts = 1 << inserts
    numberOfRanks = 10000
    file = open("evaluationInput-2^" + str(inserts) + ".txt", "w")

    file.write("1\n")
    file.write("1\n") #start with a single 1 bit

    currentBvSize = 1
    for i in range(numberOfInserts):
        index = np.random.randint(0, currentBvSize)
        bit = np.random.randint(0, 2)
        file.write("insert " + str(index) + " " + str(bit) + "\n")
        currentBvSize += 1

    for r in range(numberOfRanks):
        index = np.random.randint(0, currentBvSize)
        bit = np.random.randint(0, 2)
        file.write("rank " + str(bit) + " " + str(index) + "\n")
