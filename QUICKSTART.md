# Quick Start Guide

## Testing the buildinfo Project

### 1. Build the tools

```bash
cd /Volumes/Devel/Projects/buildinfo
make
```

This compiles `extract-buildinfo`.

### 2. Make the buildinfo script executable

```bash
chmod +x buildinfo
```

### 3. Run the test suite

```bash
make test
```

This will:
- Create a temporary test project
- Build it with buildinfo support
- Test the version flags
- Extract metadata from the binary
- Clean up

### 4. Try creating a new project

```bash
./buildinfo setup /tmp/mytest
cd /tmp/mytest
make
./bin/myapp -V
./bin/myapp --version
```

### 5. Extract metadata from the binary

```bash
./extract-buildinfo /tmp/mytest/bin/myapp
```

### 6. Install system-wide (optional)

```bash
sudo make install
```

Then use from anywhere:
```bash
buildinfo setup mynewproject
```

## Testing with Your Presign Project

To integrate buildinfo into your existing presign project:

```bash
cd /Volumes/Devel/Projects/PresignUrl/presign
/Volumes/Devel/Projects/buildinfo/buildinfo setup .
```

This will:
1. Add `buildinfo.mk` to your project
2. Create a `VERSION` file (or keep existing)
3. Add `src/buildinfo.h`
4. Show you the Makefile changes needed

Then follow the integration instructions shown.

## What You Get

After integration, your binaries will have:

1. **Runtime access to build info:**
   ```c
   #include "buildinfo.h"
   printf("%s\n", build_full_version);
   print_version_info();  // Detailed output
   ```

2. **External metadata extraction:**
   ```bash
   extract-buildinfo ./bin/presign
   # Shows all build metadata without running the binary
   ```

3. **Version tracking in git:**
   - Commit hash embedded in binary
   - Dirty working tree detection
   - Branch name included
   - Build timestamps

## Customization

### Change version format

Edit `buildinfo.mk` and modify the `GITVER` variable construction.

### Change metadata fields

Edit `buildinfo.mk` in the `generate-buildinfo` target to add/remove fields.

### Customize version output

Edit the `print_version_info()` function generation in `buildinfo.mk`.

## Troubleshooting

**"command not found: buildinfo"**
- Either use `./buildinfo` from the project directory, or run `sudo make install`

**"No .buildinfo section found"**
- Make sure you compiled with the buildinfo.o object linked in
- Check that `__attribute__((section(".buildinfo")))` is in buildinfo.c
- On some systems, unused sections may be stripped - add `__attribute__((used))` (already in template)

**"Failed to read ELF/Mach-O header"**
- Make sure you're pointing to the actual binary, not a script or text file
- Verify the file is executable: `file ./bin/myapp`
