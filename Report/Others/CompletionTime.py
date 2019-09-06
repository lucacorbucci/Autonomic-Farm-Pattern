import matplotlib.pyplot as plt
import numpy as np
import sys

time = [31.37, 38.82, 75.72, 163.06, 295.27, 548.22, 1099.97, 2154.43]
timeNOSQ = [28.62, 35.95, 69.76, 135.884, 270.16, 585.63, 1123.07, 2154.31]
timeFastFlow = [29.58, 36.87, 68.04, 135.78,
                270.09, 539.22, 1076.60, 2151.47]
x = np.arange(8)

time.reverse()
timeNOSQ.reverse()
timeFastFlow.reverse()


my_xticks = [1, 2, 4, 8, 16, 32, 64, 128]
# my_yticks = y
plt.xticks(x, my_xticks)
# plt.yticks(y, my_yticks)

plt.plot(x, timeNOSQ, label="C++ Threads with FastFlow Queue", color='black')
plt.plot(x, time, marker='', color='red',
         label="C++ Threads with Safe Queue")
plt.plot(x, timeFastFlow, marker='', color='blue',
         label="FastFlow")

plt.xlabel('Number of Workers')
plt.ylabel('Completion Time (in seconds)')

plt.title('Completion Time')
plt.tight_layout()

plt.legend()

plt.savefig("Completion_Time")
