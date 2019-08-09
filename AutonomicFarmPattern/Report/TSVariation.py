import matplotlib.pyplot as plt
import numpy as np
import sys


if(len(sys.argv) > 2):
    x = np.arange(100)
    y = []
    with open(sys.argv[1]) as file:
        for line in file:
            arrayLine = line.strip('\n')
            y.append(int(arrayLine))

    plt.axhline(y=1000, color='r', linestyle='-')

    plt.plot(x, y)

    plt.xlabel('Computed Tasks')
    plt.ylabel('Service Time')

    plt.title("Service Time Variation " + sys.argv[2])

    plt.legend()

    plt.savefig(sys.argv[2])

else:
    print("Usage: python " + sys.argv[0] + " FileName Title")
