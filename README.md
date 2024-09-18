# VirtMemTest

I wrote the first versions of VirtMemTest while working on the first Sysinternals book. The tool made it easy for me to perform a variety of 
memory operations and to observe how different Sysinternals utilities reacted to them. I eventually added CPU-stress capabilities, hung UI simulation, 
and crash-on-exit, particularly for exercising [ProcDump](https://learn.microsoft.com/en-us/sysinternals/downloads/procdump). I later used it in some of my technical presentations, and eventually posted
the first public version (with source code) back in 2013. In spite of its still-primitive MFC dialog UI and feature set (not to mention the total-trash throwaway quality
of the source code), I still find myself using it often to
invoke various scenarios, so I'm publishing this updated version.

![VirtMemTest screenshot](Screenshot/VirtMemTest.png)

## Memory Operations

The `VirtualAlloc`, `CreateFileMapping`, `_alloca` and `new byte[]` buttons perform memory allocation operations according to options you select 
in the rest of the Memory Options group box. `VirtualAlloc` simply calls the [VirtualAlloc](https://learn.microsoft.com/en-us/windows/win32/api/memoryapi/nf-memoryapi-virtualalloc) API; 
CreateFileMapping creates a named file mapping backed by the process' virtual memory; `_alloca` performs a stack allocation; and `new byte[]` uses the C++ “new” 
operator to allocate heap memory.

Set the amount of memory to allocate in the text box preceding “MB (KB for _alloca)”. Note that this is a simple app so there’s no overflow detection in the 
32-bit version if you exceed what can be represented in 32 bits. E.g., if you specify 5000 MB, it will multiply 5000 * 1024 * 1024, which 
overflows a 32-bit unsigned long and retains only the lower 32 bits (904 MB). Also note that in spite of the “MB” label, `_alloca` 
allocates KB instead of MB.

With `VirtualAlloc`, memory is reserved but not committed unless you select the “Commit” option.

Select the desired page protection for the new allocation with the Page Protection dropdown. The page protection feature isn't applied with _alloca.
Note also that the heap operations (`new[]` and corresponding `delete[]`) will face problems if the page protection does not include a "write" permission.

If you check “Free afterward”, VirtMemTest will display a message box after allocating the memory, and free the memory when you click OK. 
Otherwise it will simply leak the memory.

The `Allocated addresses` box lists the starting addresses for the allocated memory blocks.

**Post-Alloc Operations**

The "Read," "Write," "Execute existing," and "Execute NOPs" checkboxes perform the corresponding operations on the allocated memory after it has been allocated. 
This is an interesting way to observe what happens when you try to read from uncommitted memory or to write to read-only memory; you can test 
Data Execution Protection (DEP) by executing memory that has or has not been marked for execution. These operations also typically force these allocations
into working set (RAM), which won't happen if the memory is allocated but never referenced.

* "Read" tries to read whatever is in the allocated memory.
* "Write" tries to write arbitrary data to the allocated memory.
* "Execute existing" tries to execute whatever is present in that memory after allocation (quite probably random garbage, so the process is likely to 
crash with or without DEP).
* "Execute NOPs" will fill the allocation with NOP ("no operation") CPU instructions followed by a "return" operation (temporarily changing 
page protection to allow these bytes to be written there), and then try to execute that memory.

## CPU Consumption

When you click the “Run CPU Hog(s)” toggle button, VirtMemTest spins up one or more CPU-bound threads that run at the priority you 
specify in the Thread Priority dropdown until you toggle the CPU Hog(s) button back off. By default, Windows will typically assign that 
workload to what it determines to be the most appropriate logical processor at any given point in time. Typically you'll see that load randomly reassigned
to different processors. If you select `Single CPU Affinity`, all the threads will be scheduled only on the first logical processor. If you select `Thread-to-CPU affinity`,
each thread will be assigned to a separate logical processor, starting with processor group 0, processor 0, then processor 1, processor 2, etc. If the number
of threads exceeds the number of logical processors in the processor group, it continues the sequence with the next processor group. If the number of
threads to start exceeds the number of logical processors, the sequence starts over with group 0 / processor 0. (All this assumes that the process is
allowed to set thread affinity with any logical processor in any processor group.)

I originally added the "file touch" dropdown below the CPU-Hog button for demo purposes to show how to integrate Sysinternals [Process Monitor](https://learn.microsoft.com/en-us/sysinternals/downloads/procmon) 
and [ProcDump](https://learn.microsoft.com/en-us/sysinternals/downloads/procdump) by injecting ProcDump output into Procmon's event list. If you select `One-time file touch`, VirtMemTest will try to open “C:\VirtMemTest.log” 
right before going CPU-bound. If you select `Repeated file touch`, it also tries to open C:\VirtMemTest.log inside the CPU-bound loop.

## Hung UI

The `Hung UI` feature demonstrates how Windows deals with GUI apps that become non-responsive to UI events such as mouse clicks. 
Specify the number of seconds you want VirtMemTest to be non-responsive to UI events, then click `Hang this UI`. (The Desktop Window Manager
typically takes over rendering of the window after about 5 seconds, including putting “(Not Responding)” in the title bar.)

## Exception

I originally added the `Exception` feature when working on the [ProcDump](https://learn.microsoft.com/en-us/sysinternals/downloads/procdump) 
chapter of _[Troubleshooting with the Windows Sysinternals Tools](https://www.microsoftpressstore.com/store/troubleshooting-with-the-windows-sysinternals-tools-9780735684447)_.
Very simply, pick an object or fault type from the dropdown and click `Throw exception`. If you enable `Catch exeption`, VirtMemTest will catch the exception, which 
is still visible to debuggers such as ProcDump as a first-chance exception. Otherwise, the exception will remain uncaught (second chance).

## OutputDebugString

Clicking the `OutputDebugString` button writes whatever text you put in the text box to the Windows debug stream, visible to debuggers as well as to
tools like Sysinternals [DebugView](https://learn.microsoft.com/en-us/sysinternals/downloads/debugview) (dbgview.exe).

## Crash This App

Press Esc to exit the app. If the “Crash This App” option is selected, VirtMemTest will try to write to address 0 right before exit.




So:  Go have fun!  (According to some turbogeek definition of fun.)

