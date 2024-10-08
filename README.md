# RTOS From Scratch
Repository to track the implementation code for a priority-based RTOS from scratch. The main focus of this project was to understand the fundamentals of AAPCS exception entry and function calling standards to manipulate registers for successful context switching. The other focus being, to get exposure to programming on an STM32 and get more practice with bare metal programming.  

The main resource used to learn about the theory and implementation of RTOS can be found here https://youtu.be/hnj-7XwTYRI?si=bwQKHome_aMk3csP. The final goal was to showcase a demo of real-time task switching using the 4 LEDs on the board to visually represent 3 threads with deadlines and the idle state. A logic analyzer was used to inspect the behaviors of the thread to make sure the scheduler and thread logic was working properly.  

# Build and Tools
Languages: C  
MCU: STM32F407G - DISC1  
IDE: STM32CubeIDE  
Libraries: CMSIS  
Logic Analyzer: HiLetgo USB Logic Analyzer 8CH 24MHz with Sigrok's Pulse View

# Analysis of Threads
#### Logic Analyzer view of round robin with busy-wait delay:
![pulseview_2024-09-24_09-03-26](https://github.com/user-attachments/assets/39ef5784-83ba-4b42-9be9-e772e4fd8069)
The 4th digital signal represents the systick handler firing at a consistent interval. As shown in the analyzer view, the context switching always happens when the systick handler fires because the PendSV Handler is triggered through the scheduler everytime the Systick Handler is triggered. The pattern of the square waves show the round robin scheduler is working as intended.
<br />
<br />
#### Logic Analyzer view of round robin with efficient blocking using D4 as the idle thread view:
![pulseview_2024-09-24_11-04-03](https://github.com/user-attachments/assets/3a7904bb-4d63-426f-a640-1295e95b4819)
This view of the logical analyzer is testing the thread blocking implementation. As shown, the 4th digital signal is the idle thread which runs for the majority of the lifetime of the program. This analyzer view proves the LED blinky threads are blocking properly and the idle thread runs until the blinky threads reach a timeout of 0. Once the blinky threads are flagged to run again using the ready_mask bit mask, those threads are serviced and then the idle thread continues to run again.
<br />
<br />
#### Logic Analyzer view of priority based scheduling:
![pulseview_2024-09-24_14-06-57](https://github.com/user-attachments/assets/60e8d43c-74a0-457c-9f91-c4a8f696bffd)
The blinky1 (red) thread is being blocked for 20ms, and it runs for 6ms.
The blinky2 (orange) thread is being blocked for 50ms, and it runs for 18ms.
The idle thread runs whenever both threads are blocked and the systick is firing at an interval of 1ms at a time.
The square waves show the priority based scheduling is working properly as the red LED is always meeting its deadline. It is clearly shown by how blinky1 preempts the blinky2 (orange) thread at varied positions of each total run cycle of blinky2.
