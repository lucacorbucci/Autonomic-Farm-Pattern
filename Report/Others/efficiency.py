import matplotlib.pyplot as plt
import numpy as np
import sys
import matplotlib.lines as mlines


if(len(sys.argv) > 1):
    x = np.array([1, 2, 4, 8, 16, 32, 64, 128])
    time = [31.37, 38.82, 75.72, 163.06, 295.27, 548.22, 1099.97, 2154.43]
    timeNOSQ = [28.62, 35.95, 69.76, 135.884, 270.16, 585.63, 1123.07, 2154.31]
    timeFastFlow = [29.58, 36.87, 68.04, 135.78,
                    270.09, 539.22, 1076.60, 2151.47]

    tSeq = 2068.25

    time.reverse()
    EfficiencySQ = [(tSeq/(val*nw)) for nw, val in zip(x, time)]
    print EfficiencySQ

    timeNOSQ.reverse()
    EfficiencyNOSQ = [(tSeq/(val*nw)) for nw, val in zip(x, timeNOSQ)]
    print EfficiencyNOSQ

    timeFastFlow.reverse()
    EfficiencyFF = [(tSeq/(val*nw)) for nw, val in zip(x, timeFastFlow)]
    print EfficiencyFF

    my_xticks = [1, 2, 4, 8, 16, 32, 64, 128]
    ax = [1, 2, 3, 4, 5, 6, 7, 8]

    plt.tick_params(axis='x', which='major', pad=10, labelsize=8)
    plt.plot(ax, EfficiencyNOSQ, label="C++ Threads with FastFlow Queue",
             color='black', marker='.', linestyle='dotted')
    plt.plot(ax, EfficiencySQ, color='black',
             label="C++ Threads with Safe Queue", marker='*', linestyle='--')
    plt.plot(ax, EfficiencyFF, color='black',
             label="FastFlow", marker='+', linestyle='-.')

    plt.xticks(ax, my_xticks)

    plt.xlabel('Number of Workers')
    plt.ylabel('Efficiency')

    plt.title(sys.argv[1])
    plt.tight_layout()

    plt.legend()

    plt.savefig(sys.argv[1])

else:
    print("Usage: python " + sys.argv[0] + " Title")
