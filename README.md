# ChronOS64
ChronOS64 is the 64-bit version of ChronOS.

## Todo list

### Basics
- [x] GDT
- [x] IDT
- [x] Paging
- [x] Physical memory manager
- [ ] Virtual memory manager

### Debugging
- [x] Stack trace

### Timers
- [X] HPET
- [ ] PIT
- [ ] Base timer 'class'

### PCI
- [x] Enumeration
- [ ] Loading drivers

### Interrupts
- [ ] LAPIC and I/O APIC
- [ ] PIC...?

### Drivers
- [x] QEMU EDU
- [x] E1000
- [x] AHCI
- [ ] UHCI
- [ ] AC97 (maybe)

### Multitasking
- [ ] Scheduling

### Filesystems
- [x] FAT
- [ ] ext2 (maybe)
- [x] VFS (sort of)

### Network protocols
- [x] ARP
- [ ] ICMP
- [ ] UDP

### Other
- [x] AML interpreter (LAI library)
- [x] Basic heap (SHMALL)

### Future
- [ ] Userspace
- [ ] Window manager (maybe)

## Other libraries used in this project
- LAI (Lightweight AML interpreter) https://github.com/managarm/lai/tree/master
- SHMALL (Simple Heap Memory ALLocator) https://github.com/CCareaga/heap_allocator
- FatFs - Generic FAT Filesystem Module http://elm-chan.org/fsw/ff/
