# buildinfo

A simple, portable build metadata system for C projects using Makefiles.

## What is it?

buildinfo helps you track build metadata in your C projects, including:

- Git commit hash (short and full)
- Git branch name
- Build timestamp
- Compiler version and flags
- Build host and user
- Dirty working tree detection
- Base version from VERSION file

This metadata is embedded into your binary in two ways:
1. As C variables accessible at runtime (for `--version` flags)
2. In a custom ELF/Mach-O section (for external inspection without running the binary)

## Features

- **Zero dependencies** - Pure C and POSIX shell
- **Cross-platform** - Works on Linux and macOS
- **Makefile-based** - Integrates with standard Make workflows
- **Git-aware** - Automatically detects version info from git
- **Standalone** - Works without git too (falls back to timestamps)
- **Extractable** - Metadata can be read without executing the binary

## Quick Start

### Installation

```bash
make
sudo make install
```

This installs:
- `buildinfo` - Setup tool for projects
- `extract-buildinfo` - Metadata extraction utility

### Create a New Project

```bash
buildinfo setup myproject
cd myproject
make
./bin/myapp --version
```

This creates a complete C project skeleton with version tracking built in.

### Add to Existing Project

```bash
cd your-existing-project
buildinfo setup .
```

This will:
1. Detect your existing project structure
2. Add `buildinfo.mk`, `VERSION`, and `buildinfo.h`
3. Show you the Makefile changes needed

## Usage

### In Your Code

Include the header and use the version variables:

```c
#include "buildinfo.h"

// For -V flag (short version)
printf("%s\n", build_base_version);

// For --version flag (detailed info)
print_version_info();

// Or access individual fields
printf("Built: %s\n", build_timestamp);
printf("Commit: %s\n", build_commit_full);
```

### Makefile Integration

For new projects, the generated Makefile already includes everything. For existing projects:

```makefile
# Include buildinfo.mk
include buildinfo.mk

VERSION_FILE = VERSION
BUILDINFO_SRC = $(BUILDDIR)/buildinfo.c
OBJECTS = $(BUILDDIR)/yourfile.o $(BUILDDIR)/buildinfo.o

# Generate buildinfo.c before compiling
$(BUILDINFO_SRC): buildinfo.mk $(VERSION_FILE)
	$(MAKE) -f buildinfo.mk generate-buildinfo \
		BUILDDIR=$(BUILDDIR) \
		VERSION_FILE=$(VERSION_FILE) \
		CC=$(CC)

$(BUILDDIR)/buildinfo.o: $(BUILDINFO_SRC)
	$(CC) $(CFLAGS) -c $< -o $@
```

### Extracting Metadata

You can read build metadata without running the binary:

```bash
extract-buildinfo ./bin/myapp
```

Output:
```
base_version=1.0.0
full_version=main@a1b2c3d4-20251012T143052Z
commit=a1b2c3d4e5f6g7h8i9j0k1l2m3n4o5p6q7r8s9t0
commit_short=a1b2c3d4
timestamp=2025-10-12T14:30:52Z
dirty=false
build_host=mycomputer.local
build_user=username
build_os=Darwin
build_arch=arm64
compiler=Apple clang version 15.0.0
```

### Native Tools

You can also use platform-native tools:

**Linux:**
```bash
# Extract the section
objcopy --dump-section .buildinfo=/dev/stdout ./bin/myapp

# Or view in hex
readelf -x .buildinfo ./bin/myapp
```

**macOS:**
```bash
# Extract the section
otool -s __TEXT __buildinfo ./bin/myapp | tail -n +3 | xxd -r
```

## Version Format

The full version string follows this format:

```
NAME@REV[.dirty]-TIMESTAMP
```

Examples:
- `main@a1b2c3d4-20251012T143052Z` - Clean build from main branch
- `feature@b2c3d4e5.dirty-20251012T150530Z` - Dirty build from feature branch
- `v1.0.0@c3d4e5f6-20251012T160045Z` - Build from tag
- `detached@d4e5f6g7-20251012T170830Z` - Detached HEAD state

Outside git repositories, it falls back to:
```
unknown@unknown-TIMESTAMP
```

## Available Metadata Variables

| Variable | Description |
|----------|-------------|
| `build_base_version` | Version from VERSION file (e.g., "1.0.0") |
| `build_full_version` | Complete version string with git info |
| `build_commit_short` | Short git commit hash (8 chars) |
| `build_commit_full` | Full git commit hash |
| `build_timestamp` | ISO 8601 build timestamp (UTC) |
| `build_dirty` | "true" if built from dirty working tree |
| `build_host` | Hostname where built |
| `build_user` | Username who built it |
| `build_os` | Operating system (from uname -s) |
| `build_arch` | CPU architecture (from uname -m) |
| `build_compiler` | Compiler version string |

## Project Structure

After running `buildinfo setup`, your project will have:

```
myproject/
├── Makefile           # Build configuration with buildinfo integration
├── buildinfo.mk       # Build metadata generation logic
├── VERSION            # Base version number (e.g., "1.0.0")
├── src/
│   ├── main.c         # Your source code
│   └── buildinfo.h    # Build metadata header
├── build/
│   ├── buildinfo.c    # Auto-generated (do not edit)
│   ├── buildinfo.o
│   └── main.o
└── bin/
    └── myapp          # Your compiled binary
```

## Requirements

- POSIX-compatible shell (bash, sh, zsh)
- Make
- C99 compiler (gcc, clang)
- Git (optional, for version info)

## How It Works

1. **buildinfo.mk** interrogates git and the build environment
2. It generates `build/buildinfo.c` with all metadata as const strings
3. The `__attribute__((section(".buildinfo")))` directive places structured metadata in a custom section
4. Your Makefile compiles and links `buildinfo.o` into your binary
5. At runtime, your code can access the metadata via `buildinfo.h`
6. `extract-buildinfo` can read the custom section without executing the binary

## License

This is free and unencumbered software released into the public domain.

## Contributing

This is a simple, self-contained tool. If you have improvements:

1. Keep it simple
2. Maintain portability (POSIX sh, standard C99)
3. No external dependencies beyond Make and a C compiler
4. Test on both Linux and macOS

## Examples

### Example 1: Simple Version Check

```c
#include <stdio.h>
#include <string.h>
#include "buildinfo.h"

int main(int argc, char *argv[]) {
    if (argc > 1 && strcmp(argv[1], "--version") == 0) {
        print_version_info();
        return 0;
    }
    
    // Your program logic here
    return 0;
}
```

### Example 2: Version in Help Text

```c
void print_help(void) {
    printf("myapp version %s\n", build_base_version);
    printf("Usage: myapp [options]\n");
    // ...
}
```

### Example 3: Conditional Logic Based on Build

```c
#include <string.h>
#include "buildinfo.h"

int main(void) {
    if (strcmp(build_dirty, "true") == 0) {
        fprintf(stderr, "Warning: Running development build\n");
    }
    
    // Your program logic
    return 0;
}
```

## Frequently Asked Questions

**Q: Does this work without git?**  
A: Yes! If git is not available, it falls back to timestamp-based versioning.

**Q: Can I customize the version format?**  
A: Yes, edit buildinfo.mk to change the GITVER variable construction.

**Q: Does this increase binary size?**  
A: Minimally - typically a few hundred bytes for the metadata strings.

**Q: Can I use this with CMake or other build systems?**  
A: Currently only Makefile-based projects are supported.

**Q: How do I change the base version?**  
A: Edit the VERSION file. Use semantic versioning (e.g., "1.0.0", "2.1.3").

**Q: Can I strip the metadata for production builds?**  
A: Yes, use `strip` or linker flags to remove sections, though this defeats the purpose.

## See Also

- Go's version embedding: `go version -m`
- Rust's build info: `built` crate
- Git describe: `git describe --tags --always --dirty`
