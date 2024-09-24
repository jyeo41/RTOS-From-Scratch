Logic Analyzer view of round robin with busy-wait delay:
![pulseview_2024-09-24_09-03-26](https://github.com/user-attachments/assets/39ef5784-83ba-4b42-9be9-e772e4fd8069)
The 4th digital signal represents the systick handler firing at a consistent interval. As shown in the analyzer view, the context switching always happens when the systick handler fires because the PendSV Handler is triggered through the scheduler everytime the Systick Handler is triggered. The pattern of the square waves show the round robin scheduler is working as intended.


Logic Analyzer view of round robin with efficient blocking using D4 as the idle thread view:
![pulseview_2024-09-24_11-04-03](https://github.com/user-attachments/assets/3a7904bb-4d63-426f-a640-1295e95b4819)
This view of the logical analyzer is testing the thread blocking implementation.


Logic Analyzer view of priority based scheduling:
![pulseview_2024-09-24_14-06-57](https://github.com/user-attachments/assets/60e8d43c-74a0-457c-9f91-c4a8f696bffd)

The blinky1 (red) thread is being blocked for 20ms, and it runs for 6ms.
The blinky2 (orange) thread is being blocked for 50ms, and it runs for 18ms.
The idle thread runs whenever both threads are blocked and the systick is firing at an interval of 1ms at a time.
The square waves show the priority based scheduling is working properly as the red LED is always meeting its deadline. It is clearly shown by how blinky1 preempts the blinky2 (orange) thread at varied positions of each total run cycle of blinky2.
