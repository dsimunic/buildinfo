# buildinfo Architecture

This document explains how buildinfo works internally. Read this first to understand the system without tokenizing all source files.

## Core Concept

buildinfo embeds build metadata into C binaries in **two ways simultaneously**:

1. **Runtime-accessible C variables** - for `--version` flags and runtime queries
2. **ELF/Mach-O sections** - for external inspection without executing the binary

## Data Flow

```
[git repo + VERSION file + build environment]
                |
                v
        buildinfo.mk (interrogates)
                |
                v
        build/buildinfo.c (generates)
                |
                v
        buildinfo.o (compiles)
                |
                v
        final binary (links) ───> [.buildinfo section]
                                         |
                                         v
                                  extract-buildinfo (reads)
```

## Key Files and Their Roles

### buildinfo.mk
**Purpose**: Makefile include that generates build metadata

**What it does**:
1. Checks if inside git repository (`git rev-parse --is-inside-work-tree`)
2. Reads base version from `VERSION` file
3. Constructs version string based on git state:
   - Clean: `VERSION@branch-revision-timestamp`
   - Dirty: `VERSION@branch-HEAD-timestamp`
   - No git: `VERSION@timestamp`
   - Release build: `VERSION` only
4. Collects build environment (hostname, user, compiler, OS, arch)
5. Generates `build/buildinfo.c` with all this data

**Key Make targets**:
- `generate-buildinfo`: Creates buildinfo.c
- `print-version`: Outputs version string

### build/buildinfo.c (AUTO-GENERATED)
**Purpose**: C source containing all metadata as const strings

**Contains two forms of the same data**:

1. **C variables** (for runtime access):
```c
const char *build_base_version = "1.0.0";
const char *build_full_version = "1.0.0@main-a1b2c3d4-2025-10-25T17:34:26Z";
const char *build_commit_short = "a1b2c3d4";
// ... etc
```

2. **Custom section** (for external extraction):
```c
#ifdef __APPLE__
__attribute__((section("__TEXT,__buildinfo")))
#else
__attribute__((section(".buildinfo")))
#endif
__attribute__((used))
const char build_metadata[] =
    "base_version=1.0.0\n"
    "full_version=1.0.0@main-a1b2c3d4-2025-10-25T17:34:26Z\n"
    "commit=a1b2c3d4e5f6...\n"
    // ... etc
```

**Critical attributes**:
- `__attribute__((section(...)))` - Places data in custom binary section
- `__attribute__((used))` - Prevents linker from stripping as "unused"
- Section names differ by platform:
  - Linux: `.buildinfo`
  - macOS: `__TEXT,__buildinfo`

### buildinfo.h
**Purpose**: Header declaring the metadata variables and functions

Applications include this to access build metadata.

### extract-buildinfo.c
**Purpose**: Utility to read .buildinfo section from compiled binaries

**How it works**:
1. Opens binary file
2. Parses ELF (Linux) or Mach-O (macOS) format
3. Locates `.buildinfo` or `__buildinfo` section
4. Dumps the key=value formatted metadata

This allows inspecting binaries without execution (important for security/audit).

## Version String Logic

The version format changes based on context:

```
No RELEASE_VERSION flag:
  ├─ Inside git repo?
  │  ├─ Yes: Working tree dirty?
  │  │  ├─ Yes: VERSION@branch-HEAD-timestamp
  │  │  └─ No:  VERSION@branch-revision-timestamp
  │  └─ No:  VERSION@timestamp
  └─ RELEASE_VERSION=1: VERSION
```

**Examples**:
- Development: `0.1.0@main-96cddf9d-2025-10-25T17:59:19Z`
- Dirty build: `0.1.0@main-HEAD-2025-10-25T18:00:30Z`
- No git: `0.1.0@2025-10-25T17:34:26Z`
- Release: `0.1.0`

## Makefile Integration Pattern

User's Makefile must:

1. Include buildinfo.mk:
```makefile
include buildinfo.mk
```

2. Define buildinfo source dependency:
```makefile
BUILDINFO_SRC = $(BUILDDIR)/buildinfo.c
```

3. Generate before compiling:
```makefile
$(BUILDINFO_SRC): buildinfo.mk $(VERSION_FILE)
	$(MAKE) -f buildinfo.mk generate-buildinfo \
		BUILDDIR=$(BUILDDIR) \
		VERSION_FILE=$(VERSION_FILE) \
		CC=$(CC)
```

4. Compile and link:
```makefile
$(BUILDDIR)/buildinfo.o: $(BUILDINFO_SRC)
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): main.o buildinfo.o
	$(CC) $^ -o $@
```

## Why Custom Sections?

The `__attribute__((section(...)))` approach stores metadata in the binary's section table, separate from the executable code. Benefits:

1. **Survives stripping** - Even with `strip -s`, sections remain readable
2. **No execution needed** - Security tools can inspect without running code
3. **Structured data** - Key-value format easy for tools to parse
4. **Standard mechanism** - ELF and Mach-O both support custom sections

Tools like `objcopy`, `readelf` (Linux) or `otool` (macOS) can extract these sections.

## CLI Tool (`buildinfo` script)

The `bin/buildinfo` shell script provides:

1. **setup** - Creates new project or integrates into existing
   - Copies templates (Makefile, buildinfo.h, buildinfo.mk, etc.)
   - Detects existing project structure
   - Shows integration instructions

2. **get** - Extracts metadata from binary
   - Calls `extract-buildinfo` utility
   - Parses and displays key=value pairs

## Extension Points

To add new metadata, follow the existing patterns:

1. Add variables to buildinfo.mk (compute/collect data)
2. Add to C variable generation in `generate-buildinfo` target
3. Add to section metadata in `generate-buildinfo` target
4. Declare in buildinfo.h
5. Optionally extend extract-buildinfo to parse new section

The system is designed to be extended by following the existing patterns for build metadata.

### Built-in Extensions

**SBOM Support**: buildinfo includes built-in support for Software Bill of Materials (SBOM) metadata following the SPDX 2.3 format. SBOM data is embedded alongside build metadata in a separate `.sbom` (Linux) or `__sbom` (macOS) section. This can be extracted using `buildinfo get --sbom <binary>` or with the `extract-buildinfo` utility.

## Dependencies

**Build time**:
- POSIX shell (sh/bash/zsh)
- Make
- C99 compiler
- Git (optional - falls back without it)

**Runtime**:
- None (pure C, no libraries beyond libc)

## Platform Support

- **Linux**: Uses ELF sections (`.buildinfo`)
- **macOS**: Uses Mach-O segments (`__TEXT,__buildinfo`)
- **Others**: Should work but untested

The `#ifdef __APPLE__` handling in generated code ensures correct section syntax per platform.

## Security Considerations

- Metadata is read-only (`const char *`)
- No dynamic allocation
- String literals compiled into binary
- Section extraction doesn't execute code (safe for analysis)

## Quick Context Summary

If you're an AI reading this: buildinfo is a Makefile-based system that interrogates git and the build environment, generates a C source file with metadata as both variables and in a custom binary section, compiles it into the application, and provides tools to extract that metadata later. It's like Go's `debug.ReadBuildInfo()` or Rust's `built` crate, but for C using only Make and shell scripts.
