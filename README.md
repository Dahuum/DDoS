# Disk Emulator

Virtual disk layer for DDoS. Files as block devices.
```
┌─────────────────┐
│   command.c     │  ← main
└─────────────────┘
         │
         ▼
┌─────────────────┐
│     disk.c      │  ← disk emulator
│ dread()/dwrite()│
└─────────────────┘
         │
         ▼
┌─────────────────┐
│     osapi.c     │  ← OS abstraction
└─────────────────┘
         │
         ▼
┌─────────────────┐
│  Unix system    │  ← real I/O
│ read()/write()  │
└─────────────────┘
```

**Block layout:**
```
disk.1 (32MB):
┌────────┬────────┬────────┬─────┐
│Block 0 │Block 1 │Block 2 │ ... │
│ 512B   │ 512B   │ 512B   │     │
└────────┴────────┴────────┴─────┘

Block N = Byte (N × 512)
```

**Usage:**
```c
disk *dd = dattach(1); // disk attach, mount it
dread(dd, &buf, 500);  // disk read 
dwrite(dd, &buf, 0);   // dist write
ddetach(dd);           // disk detach, mhm unmount it
```

**Build:**
```bash
dd if=/dev/zero of=disk.1 bs=512 count=65535 # create disk in drives/
make        # from command/
make clean  # Clean /command
make fclean # osapi/ command/
```
