import matplotlib.pyplot as plt
import numpy as np
import sys


if(len(sys.argv) > 2):
    x = np.arange(1000)
    TS = []
    nWorker = []
    with open(sys.argv[1]) as file:
        for line in file:
            line = line.strip('\n')
            arrayLine = line.split(" ")
            TS.append(int(arrayLine[0]))
            nWorker.append(int(arrayLine[1]))

    sum = 0
    # Compute avg
    for item in TS:
        sum += item

    print(round(sum/1000))

    fig, ax1 = plt.subplots()
    color = 'black'
    ax1.set_xlabel('Computed Task')
    ax1.set_ylabel('Service Time', color=color)
    ax1.plot(x, TS, color=color)
    ax1.tick_params(axis='y', labelcolor=color)
    plt.title(sys.argv[2])

    ax1.axhline(y=100, color='gray', linestyle='-')
    fig.savefig(sys.argv[2] + "_OnlyTS")

    fig2, ax = plt.subplots()
    color = 'black'
    ax.set_xlabel('Computed Task')
    ax.set_ylabel('Service Time', color=color)
    ax.plot(x, TS, color=color)
    ax.tick_params(axis='y', labelcolor=color)

    ax2 = ax.twinx()

    color = 'black'
    ax2.set_ylabel('Workers', color=color)
    ax2.plot(x, nWorker, color='gray', linestyle='--')
    ax2.tick_params(axis='y', labelcolor=color)
    plt.title(sys.argv[2])
    plt.tight_layout()

    fig2.savefig(sys.argv[2] + "_Full")

else:
    print("Usage: python " + sys.argv[0])
