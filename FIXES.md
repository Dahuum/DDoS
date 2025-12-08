# Critical Bug Fixes

## Summary
Fixed critical bugs in filesystem implementation: uninitialized variables, logic errors, and incorrect drive letter calculations that could cause filesystem corruption and crashes.

---

## Bug 1: Uninitialized Variable in `mkbitmap()`
**File:** `osapi/fs.c` Line 8

### Error:
```c
int16 size, n, x, index;  // 'index' never initialized
```

Variable `index` was declared but never initialized, causing it to contain garbage values. When used in `setbit($1 bm, index, true)` at line 37, this would set random bits in the bitmap, corrupting the block allocation tracking.

### Impact:
- Random blocks marked as allocated
- Potential data corruption
- Unpredictable filesystem behavior

### Fix:
```c
int16 size, n, x, index = 0;  // Initialize to 0
```

---

## Bug 2: Empty Statement After Condition in `dattach()`
**File:** `osapi/disk.c` Line 66

### Error:
```c
if ((drive==1) || (drive==2));  // ← Semicolon creates empty statement
else return (disk*)0;
```

The semicolon after the `if` condition creates an empty statement, making the `else` branch always execute regardless of the condition. This prevents any disk from being attached.

### Impact:
- No disk can be successfully attached
- Filesystem cannot be mounted
- Complete system failure

### Fix:
```c
if ((drive==1) || (drive==2))
    ;  // Remove this line entirely, or add proper body
else 
    return (disk*)0;

// Better fix:
if (drive != 1 && drive != 2)
    return (disk*)0;
```

---

## Bug 3: Incorrect Drive Letter Calculation in `mkpath()`
**File:** `osapi/fs.c` Line 755

### Error:
```c
drive = (c - 0x62);  // 0x62 = 'b'
```

This calculates drive numbers incorrectly:
- 'a' → -1 (invalid)
- 'b' → 0 (invalid) 
- 'c' → 1 ✓
- 'd' → 2 ✓

The calculation works by accident for 'c' and 'd' but would fail for 'a' or 'b'.

### Impact:
- Drive 'a:' produces negative drive number
- Drive 'b:' produces 0 (invalid)
- Potential array out-of-bounds access

### Fix:
```c
drive = (c - 'a' + 1);  // 'a'→1, 'b'→2, 'c'→3, 'd'→4
// Or for your current system (c: and d: only):
drive = (c - 'b');  // 'c'→1, 'd'→2
```

---

## Bug 4: Debug Print Left in Production Code
**File:** `osapi/fs.c` Line 745

### Error:
```c
printf("\033[1m" "Daba blati n3ref had str lidakhel -> '%s'\033[0m\n", $c str);
```

Debug print statement left in production code, cluttering output.

### Impact:
- Unprofessional output
- Performance overhead (minimal)
- Confusing for users

### Fix:
```c
// Remove line, or wrap in debug macro:
#ifdef DEBUG
printf("DEBUG: Parsing path '%s'\n", $c str);
#endif
```

---

## Bug 5: Debug Print in `findchar()`
**File:** `osapi/omnistd.c` Line 122

### Error:
```c
printf("returned value is = '%s'\n\n", (*p == needle)? p: $1 0);
```

Another debug print left in production code.

### Fix:
Remove the line entirely.

---

## Bug 6: Indirect Block Always Uses `pointers[0]` in `makedir()`
**File:** `osapi/osapi.c` Lines 318-320

### Error:
```c
zero($1 &bl.data, Blocksize);
bl.pointers[0] = idx2;  // Always overwrites first slot!
dwrite(pp->fs->dd, &bl.data, blockno);
```

When adding entries to the indirect block, it always writes to `pointers[0]`, overwriting previous entries instead of finding the next free slot.

### Impact:
- Can only have 9 entries per directory (8 direct + 1 indirect)
- Should support 264 entries (8 direct + 256 indirect)
- Data loss when adding 10th+ entry

### Fix:
```c
// Read existing indirect block if it exists
if (!ino->indirect) {
    blockno = bitmapalloc(pp->fs, pp->fs->bitmap);
    ino->indirect = blockno;
    zero($1 &bl.data, Blocksize);
} else {
    blockno = ino->indirect;
    dread(pp->fs->dd, &bl.data, blockno);  // Read existing data
}

// Find first free slot
for (n=0; n<PtrPerBlock; n++) {
    if (!bl.pointers[n]) {
        bl.pointers[n] = idx2;  // Use free slot, not always [0]
        break;
    }
}

if (n >= PtrPerBlock)
    reterr(ErrDirFull);  // Directory full (264 entries)

dwrite(pp->fs->dd, &bl.data, blockno);
```

---

## Bug 7: Typo in Function Name Declaration
**File:** `osapi/fs.h` Line 123

### Error:
```c
internal void fsunmout(filesystem*);  // Missing 'n'
```

Function is defined as `fsunmount()` in fs.c but declared as `fsunmout()` in header.

### Impact:
- Compilation warning/error
- Linker error if function is called

### Fix:
```c
internal void fsunmount(filesystem*);  // Add missing 'n'
```

---

## Bug 8: Potential Stack Overflow in `parsepath()`
**File:** `osapi/fs.c` Lines 685-688

### Error:
```c
if (idx >= (DirDepth)) {
    *pptr->dirpath[idx] = (int8)0;
    return true;  // Returns true on overflow!
}
```

When path depth exceeds `DirDepth` (16), function returns `true` indicating success, but the path is truncated.

### Impact:
- Silent path truncation
- Wrong directory accessed
- Security issue (path traversal)

### Fix:
```c
if (idx >= (DirDepth)) {
    *pptr->dirpath[idx] = (int8)0;
    return false;  // Return false to indicate error
}
```

---

## Testing Recommendations

After applying fixes, test:

1. **Bitmap allocation:** Format disk, create multiple files, verify bitmap
2. **Disk attachment:** Test attaching drives 1 and 2
3. **Drive letters:** Test paths with 'c:' and 'd:'
4. **Large directories:** Create 20+ files in one directory
5. **Deep paths:** Create nested directories 10+ levels deep

---

## Files Modified
- `osapi/fs.c` (4 bugs)
- `osapi/disk.c` (1 bug)
- `osapi/osapi.c` (1 bug)
- `osapi/omnistd.c` (1 bug)
- `osapi/fs.h` (1 bug)
