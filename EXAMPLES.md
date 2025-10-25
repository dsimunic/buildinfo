# Real-World Examples

## Example 1: Simple CLI Tool with Version Flags

```c
#include <stdio.h>
#include <string.h>
#include "buildinfo.h"

void print_usage(const char *prog) {
    printf("Usage: %s [OPTIONS] <input>\n", prog);
    printf("\nOptions:\n");
    printf("  -V            Show version\n");
    printf("  --version     Show detailed build info\n");
    printf("  -h, --help    Show this help\n");
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }

    // Handle version flags
    if (strcmp(argv[1], "-V") == 0) {
        printf("%s\n", build_base_version);
        return 0;
    }
    
    if (strcmp(argv[1], "--version") == 0) {
        print_version_info();
        return 0;
    }
    
    if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
        print_usage(argv[0]);
        return 0;
    }

    // Your actual program logic here
    printf("Processing: %s\n", argv[1]);
    
    return 0;
}
```

## Example 2: Daemon with Logging

```c
#include <stdio.h>
#include <syslog.h>
#include "buildinfo.h"

void log_startup_info(void) {
    syslog(LOG_INFO, "Starting daemon version %s", build_full_version);
    syslog(LOG_INFO, "Built: %s on %s", build_timestamp, build_host);
    syslog(LOG_INFO, "Commit: %s", build_commit_full);
    
    if (strcmp(build_dirty, "true") == 0) {
        syslog(LOG_WARNING, "Running from dirty build - not for production!");
    }
}

int main(void) {
    openlog("mydaemon", LOG_PID | LOG_CONS, LOG_DAEMON);
    
    log_startup_info();
    
    // Your daemon logic here
    
    closelog();
    return 0;
}
```

## Example 3: Library with Version Check

```c
// mylib.h
#ifndef MYLIB_H
#define MYLIB_H

typedef struct {
    const char *version;
    const char *commit;
    const char *build_date;
} version_info_t;

void mylib_get_version(version_info_t *info);
int mylib_check_version(const char *required_version);

#endif

// mylib.c
#include "mylib.h"
#include "buildinfo.h"
#include <string.h>

void mylib_get_version(version_info_t *info) {
    info->version = build_base_version;
    info->commit = build_commit_short;
    info->build_date = build_timestamp;
}

int mylib_check_version(const char *required_version) {
    return strcmp(build_base_version, required_version) >= 0;
}
```

## Example 4: Debug Mode Detection

```c
#include <stdio.h>
#include <string.h>
#include "buildinfo.h"

int is_debug_build(void) {
    return strcmp(build_dirty, "true") == 0;
}

void conditional_features(void) {
    if (is_debug_build()) {
        printf("Debug features enabled\n");
        // Enable extra logging, assertions, etc.
    } else {
        printf("Production mode\n");
    }
}

int main(void) {
    printf("Version: %s\n", build_full_version);
    
    if (is_debug_build()) {
        fprintf(stderr, "WARNING: Debug build - not for production use!\n");
    }
    
    conditional_features();
    
    return 0;
}
```

## Example 5: HTTP Server with Version Endpoint

```c
#include <stdio.h>
#include <string.h>
#include "buildinfo.h"

// Pseudo-code for HTTP handler
void handle_version_endpoint(http_request_t *req, http_response_t *resp) {
    char json[1024];
    
    snprintf(json, sizeof(json),
        "{\n"
        "  \"version\": \"%s\",\n"
        "  \"commit\": \"%s\",\n"
        "  \"built\": \"%s\",\n"
        "  \"compiler\": \"%s\",\n"
        "  \"platform\": \"%s/%s\"\n"
        "}",
        build_base_version,
        build_commit_full,
        build_timestamp,
        build_compiler,
        build_os,
        build_arch
    );
    
    http_response_set_body(resp, json);
    http_response_set_header(resp, "Content-Type", "application/json");
}

int main(void) {
    // Setup HTTP server
    http_server_t *server = http_server_create(8080);
    http_server_add_route(server, "/version", handle_version_endpoint);
    
    printf("Starting server %s\n", build_full_version);
    http_server_run(server);
    
    return 0;
}
```

## Example 6: Crash Reporter

```c
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include "buildinfo.h"

void crash_handler(int sig) {
    fprintf(stderr, "\n=== CRASH REPORT ===\n");
    fprintf(stderr, "Signal: %d\n", sig);
    fprintf(stderr, "Version: %s\n", build_full_version);
    fprintf(stderr, "Commit: %s\n", build_commit_full);
    fprintf(stderr, "Built: %s\n", build_timestamp);
    fprintf(stderr, "Platform: %s/%s\n", build_os, build_arch);
    fprintf(stderr, "Compiler: %s\n", build_compiler);
    fprintf(stderr, "===================\n");
    
    // Write to crash log file
    FILE *f = fopen("crash.log", "a");
    if (f) {
        fprintf(f, "Crash at %s - version %s (%s)\n",
                __DATE__, build_full_version, build_commit_short);
        fclose(f);
    }
    
    exit(1);
}

int main(void) {
    signal(SIGSEGV, crash_handler);
    signal(SIGABRT, crash_handler);
    
    // Your program logic
    
    return 0;
}
```

## Example 7: Build Environment Validation

```c
#include <stdio.h>
#include <string.h>
#include "buildinfo.h"

int validate_build_environment(void) {
    int errors = 0;
    
    // Check if built on the right host
    if (strstr(build_host, "buildserver") == NULL) {
        fprintf(stderr, "WARNING: Not built on official build server\n");
        fprintf(stderr, "Built on: %s\n", build_host);
        errors++;
    }
    
    // Check if built from clean working tree
    if (strcmp(build_dirty, "true") == 0) {
        fprintf(stderr, "ERROR: Built from dirty working tree\n");
        errors++;
    }
    
    // Check compiler version
    if (strstr(build_compiler, "gcc") == NULL && 
        strstr(build_compiler, "clang") == NULL) {
        fprintf(stderr, "WARNING: Unknown compiler: %s\n", build_compiler);
    }
    
    return errors == 0;
}

int main(void) {
    printf("Validating build environment...\n");
    
    if (!validate_build_environment()) {
        fprintf(stderr, "Build validation failed!\n");
        return 1;
    }
    
    printf("Build validation passed\n");
    printf("Version: %s\n", build_full_version);
    
    return 0;
}
```

## Example 8: Multi-Binary Project

For projects with multiple binaries sharing build info:

```makefile
# Shared buildinfo for all binaries
BUILDINFO_SRC = $(BUILDDIR)/buildinfo.c
BUILDINFO_OBJ = $(BUILDDIR)/buildinfo.o

# Generate once, use in all binaries
$(BUILDINFO_SRC): buildinfo.mk $(VERSION_FILE)
	$(MAKE) -f buildinfo.mk generate-buildinfo \
		BUILDDIR=$(BUILDDIR) VERSION_FILE=$(VERSION_FILE) CC=$(CC)

$(BUILDINFO_OBJ): $(BUILDINFO_SRC)
	$(CC) $(CFLAGS) -c $< -o $@

# Use in multiple binaries
$(BINDIR)/tool1: $(BUILDDIR)/tool1.o $(BUILDINFO_OBJ)
	$(CC) $^ -o $@

$(BINDIR)/tool2: $(BUILDDIR)/tool2.o $(BUILDINFO_OBJ)
	$(CC) $^ -o $@

$(BINDIR)/tool3: $(BUILDDIR)/tool3.o $(BUILDINFO_OBJ)
	$(CC) $^ -o $@
```

Now all three tools share the same build metadata!

## Example 9: Integration Testing

```bash
#!/bin/bash
# test-version.sh - Validate version information in binary

BINARY="./bin/myapp"

# Test 1: Binary should respond to -V
VERSION=$($BINARY -V)
if [ -z "$VERSION" ]; then
    echo "FAIL: No version output"
    exit 1
fi

# Test 2: Extract metadata
META=$(extract-buildinfo $BINARY)
if [ -z "$META" ]; then
    echo "FAIL: Could not extract metadata"
    exit 1
fi

# Test 3: Verify commit hash format
COMMIT=$(echo "$META" | grep "^commit=" | cut -d= -f2)
if [[ ! "$COMMIT" =~ ^[0-9a-f]{40}$ ]]; then
    echo "FAIL: Invalid commit hash: $COMMIT"
    exit 1
fi

# Test 4: Verify timestamp format
TIMESTAMP=$(echo "$META" | grep "^timestamp=" | cut -d= -f2)
if [[ ! "$TIMESTAMP" =~ ^[0-9]{4}-[0-9]{2}-[0-9]{2}T[0-9]{2}:[0-9]{2}:[0-9]{2}Z$ ]]; then
    echo "FAIL: Invalid timestamp: $TIMESTAMP"
    exit 1
fi

echo "PASS: All version checks passed"
```

## Example 10: CI/CD Integration

```yaml
# .github/workflows/build.yml
name: Build

on: [push, pull_request]

jobs:
  build:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v2
        with:
          fetch-depth: 0  # Full history for git info
      
      - name: Build
        run: make
      
      - name: Check version info
        run: |
          ./bin/myapp --version
          ./extract-buildinfo ./bin/myapp
      
      - name: Verify clean build
        run: |
          # Fail if built from dirty tree
          if ./extract-buildinfo ./bin/myapp | grep -q "dirty=true"; then
            echo "ERROR: Binary shows dirty build"
            exit 1
          fi
      
      - name: Upload binary
        uses: actions/upload-artifact@v2
        with:
          name: myapp-${{ github.sha }}
          path: bin/myapp
```
