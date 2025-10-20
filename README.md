```ts
┌─────────────────┐
│   command.c     │  ← Your main program
│   calls laod()  │
└─────────────────┘
         │
         ▼
┌─────────────────┐
│     osapi.c     │  ← Your custom I/O library
│  laod()/store() │
└─────────────────┘
         │
         ▼
┌─────────────────┐
│  Unix system    │  ← Real file operations
│ read()/write()  │
└─────────────────┘

```
