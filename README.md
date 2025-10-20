```ts
┌─────────────────┐
│   command.c     │  ← main program
│   calls laod()  │
└─────────────────┘
         │
         ▼
┌─────────────────┐
│     osapi.c     │  ← custom I/O library
│  laod()/store() │
└─────────────────┘
         │
         ▼
┌─────────────────┐
│  Unix system    │  ← Real file operations
│ read()/write()  │
└─────────────────┘

```

```bash
cd command
make          # Build the project
make clean    # Clean the build files
make all      # Rebuild everything
make clean-all # Clean project and sub-project
```
