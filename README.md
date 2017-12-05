# Event-Driven-Completely-Fair-Scheduler-Simulator-
Completely Fair Scheduler is Linux kernel's main scheduling algorithm, and this program simulates this process in an event driven manner

loadgen.c can be used to create a random input file like this:
loadgen <N> <avg_start_time> <avg_num_bursts> <avg_cpu_len> <avg_io_len> <wordloadfile>

<N> is the number of processes. The mean cpu burst length for a process is
<avg_cpu_len> and the mean io burst length for a process is <avg_io_len>. The cpu
and io burst lengths are exponentially distributed. The number of cpu bursts that a
process has is also exponentially distributed with a mean of <avg_num_bursts>.
The number of io bursts for a process is one less than the number of cpu burts. The
average time that passes before a process is started is <avg_start_time>. That is also
exponentially distributed. Time stats at 0. The <workdloadfile> parameter is the
output filename into which the information will be printed.
  
Then the cfs program can be called upon the workload file like this:
cfs <workloadfile> <outputfile>
  
The output file will contain information about what has happened in the cpu during
the simulation time. This is like a log file. For each continuous time interval during
which the cpu is running a different job or cpu is idle, the file will contain the
information in the following format: <pid> <duration>. This indicates that process
<pid> used the cpu for <duration> amount of time. If cpu was idle for some time,
then the respective output line will be: idle <duration>.
  
After a process finsihes in the simulator a simple statistic line is printed to standard output.
The statistics output will be in the following format:
<pid> <prio> <starttime> <finishtime> <turnaround> <waittime> <responsetime>
  
To read more on the project and how it works consult the uploaded project description pdf
