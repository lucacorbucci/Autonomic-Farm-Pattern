# Parallel and Distributed Systems: Paradigms and Models.

## Autonomic-Farm-Pattern

### Implementation of the project using Classic Threads

#### Project structure

The project is divided into two folders:

- Docs: here you can find the documentation generate using Doxygen
- src: here you can find the source code of the Autonomic-Farm-Pattern

#### Implementation

To implement the Autonomic-Farm-Pattern i created the following components:

- Emitter
- Worker
- Collector

For each of the described components i created a .hpp file located in the path /ClassicThreads/src/AutonomicFarm.
The communication between the components is implemented using the Boost C++ Library. In particular i used the
boost::lockfree::queue and the boost::lockfree::spsc_queue because i needed a lock free queue to avoid the system calls
that i would have had using the locks.

#### Example

To test this library you can use the file main.cpp.
In particular to create the AutonomicFarm you can use this code:

```
// Create the AutonomicFarm
AutonomicFarm<int> f = AutonomicFarm<int>(nWorker, fib, tsGoal, inputVector);
// Start the execution of the AutonomicFarm
f.start();
```

You should pass the following paramers to the constructor method:

- The initial number of workers
- The function to be computed
- The ideal service time that you want to achieve
- A vector with the input for the function to be computed
