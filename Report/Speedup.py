import matplotlib.pyplot as plt
import numpy as np
import sys


if(len(sys.argv) > 1):
    x = np.array([1, 2, 4, 8, 16, 32, 64, 128])
    time = [74.9, 97.17, 185.90, 357.04, 707.83, 1499.85, 2843.45, 5692.25]
    # with open(sys.argv[1]) as file:
    #     for line in file:
    #         arrayLine = line.strip('\n')
    #         time.append(int(int(arrayLine)/1000000))

    tSeq = time[-1]

    y = [tSeq/val for val in time]
    y = [round(val, 2) for val in y]
    y.reverse()

    print y

    my_xticks = [1, 2, 4, 8, 16, 32, 64, 128]
    my_yticks = y
    plt.xticks(x, my_xticks)
    plt.yticks(y, my_yticks)

    plt.plot(x, y)

    plt.xlabel('Number of Workers')
    plt.ylabel('Speedup')

    plt.title(sys.argv[1])

    plt.legend()

    plt.savefig(sys.argv[1])

else:
    print("Usage: python " + sys.argv[0] + " Title")
