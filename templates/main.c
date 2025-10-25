#include <stdio.h>
#include <string.h>
#include "buildinfo.h"

void print_usage(const char *prog) {
    printf("Usage: %s [OPTIONS]\n", prog);
    printf("\nOptions:\n");
    printf("  -V                 Show version number\n");
    printf("  --version          Show detailed version information\n");
    printf("  -h, --help         Show this help message\n");
}

int main(int argc, char *argv[]) {
    if (argc > 1) {
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
    }
    
    printf("Hello from your new C project!\n");
    printf("Run with --version to see build information.\n");
    
    return 0;
}
