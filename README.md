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
disk *dd = dattach(1);
dread(dd, &buf, 500);
dwrite(dd, &buf, 0);
ddetach(dd);
```

**Build:**
```bash
make        # Build
make clean  # Clean
make fclean # Clean all
```

**Create disk:**
```bash
dd if=/dev/zero of=disk.1 bs=512 count=65535
```
