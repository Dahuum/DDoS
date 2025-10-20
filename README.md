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
