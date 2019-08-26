import matplotlib.pyplot as plt
import numpy as np
import sys


if(len(sys.argv) > 2):
    x = np.array([1, 2, 4, 8, 16, 32, 64, 128])
    time = []
    with open(sys.argv[1]) as file:
        for line in file:
            arrayLine = line.strip('\n')
            time.append(int(int(arrayLine)/1000000))

    tSeq = time[-1]

    y = [tSeq/val for val in time]
    y.reverse()

    print y

    my_xticks = [1, 2, 4, 8, 16, 32, 64, 128]
    my_yticks = y
    plt.xticks(x, my_xticks)
    plt.yticks(y, my_yticks)

    plt.plot(x, y)

    plt.xlabel('Number of Workers')
    plt.ylabel('Speedup')

    plt.title("Speedup " + sys.argv[2])

    plt.legend()

    plt.savefig(sys.argv[2])

else:
    print("Usage: python " + sys.argv[0] + " FileName Title")
