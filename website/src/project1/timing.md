## CPU Idle Time

As observed in the above logic analyzer timing diagrams, the CPU is idle for the majority of the time. For both the base and remote station, we are only executing three tasks, each of which use the CPU for a short amount of time. Using the average execution time of each task and its scheduled period for the TTS, we can calculate what percentage of the time the CPU is actually doing useful work.

The calcuation for CPU utilization is as follows

\begin{equation*}
  \text{CPU utilization} = \frac{\text{Computation time}}{\text{Period}}
\end{equation*}

We will use the common period of 1 second = 1000 milliseconds in the following calculations. Since most tasks run multiple times within this time frame, the equation becomes

\begin{equation*}
  \text{CPU utilization} = \frac{\text{Computation time} \times \text{Frequency}}{\text{Period}}
\end{equation*}

### Base Station

Using the data from the [above base station section](#base-station), we get the following calculation

\begin{align*}
  \text{CPU utilization} & = \frac{\text{Computation time} \times \text{Frequency}}{\text{Period}} \\
                           & = \frac{(0.65 \cdot 20) + (0.05 \cdot 20) + (12 \cdot 2)}{1000} \\
                           & = 3.8\%
\end{align*}

On the base station the CPU is only being used 3.8% of the time.

### Remote Station

Using the data from the [above remote station section](#remote-station), we get the following calculation

\begin{align*}
  \text{CPU utilization} & = \frac{\text{Computation time} \times \text{Frequency}}{\text{Period}} \\
                           & = \frac{(0.08 \cdot 20) + (0.25 \cdot 20) + (0.15 \cdot 10)}{1000} \\
                           & = 0.81\%
\end{align*}

On the remote station the CPU is only being used 0.81% of the time.
