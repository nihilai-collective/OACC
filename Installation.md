# OACC Build Guide

## Quick Start

### Linux / macOS

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . -j$(nproc)
```

### Windows (Visual Studio)

```cmd
mkdir build && cd build
cmake .. -G "Visual Studio 17 2022"
cmake --build . --config Release
```

## Build Options

| Option | Default | Description |
|--------|---------|-------------|
| `OACC_GENERATE_ASSEMBLY` | `ON` | Generate assembly output |
| `OACC_VERBOSE_BUILD` | `OFF` | Enable verbose build messages |

## Compiler Requirements

- **GCC:** 12.0+
- **Clang:** 15.0+
- **MSVC:** Visual Studio 2022 (17.0+)
- **C++ Standard:** C++23

## Build Output

```
build/
├── bin/
│   └── oacc                    # Executable
└── assembly_output/
    └── oacc.s                  # Assembly output (if enabled)
```

## Running the Example

```bash
./bin/oacc
```

## Viewing the Assembly

```bash
# Linux/macOS
less assembly_output/oacc.s

# Windows
notepad assembly_output\oacc.s
```

Search for `main:` (GCC/Clang) or `main PROC` (MSVC) to see the compiled output.

---

* Copyright 2026 Nihilai Collective Corp
* Licensed under the Apache License, Version 2.0
