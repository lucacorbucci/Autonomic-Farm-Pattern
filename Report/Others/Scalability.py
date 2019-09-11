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

    tSeq = time[-1]

    y = [tSeq/val for val in time]
    y = [round(val, 2) for val in y]
    y.reverse()

    print y

    tSeq = timeNOSQ[-1]
    y2 = [tSeq/val for val in timeNOSQ]
    y2 = [round(val, 2) for val in y2]
    y2.reverse()

    print y2

    tSeq = timeFastFlow[-1]
    y3 = [tSeq/val for val in timeFastFlow]
    y3 = [round(val, 2) for val in y3]
    y3.reverse()

    print y3

    my_xticks = [1, 2, 4, 8, 16, 32, 64, 128]
    ax = [1, 2, 3, 4, 5, 6, 7, 8]

    fig, aa = plt.subplots()
    line = mlines.Line2D([0, 1], [0, 1], color='black',
                         label="Ideal")
    transform = aa.transAxes
    line.set_transform(transform)
    aa.add_line(line)

    plt.tick_params(axis='x', which='major', pad=10, labelsize=8)
    plt.plot(ax, y2, label="C++ Threads with FastFlow Queue",
             color='black', marker='.', linestyle='dotted')
    plt.plot(ax, y, color='gray', label="C++ Threads with Safe Queue",
             marker='*', linestyle='--')
    plt.plot(ax, y3, color='black', label="FastFlow",
             marker='+', linestyle='-.')

    plt.xticks(ax, my_xticks)

    plt.xlabel('Number of Workers')
    plt.ylabel('Scalability')

    plt.title(sys.argv[1])
    plt.tight_layout()

    plt.legend()

    plt.savefig(sys.argv[1])

else:
    print("Usage: python " + sys.argv[0] + " Title")
