#ifndef BUILDINFO_H
#define BUILDINFO_H

/* Build metadata - generated at compile time by buildinfo.mk */
extern const char *build_base_version;
extern const char *build_full_version;
extern const char *build_commit_short;
extern const char *build_commit_full;
extern const char *build_timestamp;
extern const char *build_dirty;
extern const char *build_host;
extern const char *build_user;
extern const char *build_os;
extern const char *build_arch;
extern const char *build_compiler;

/* Structured metadata in custom ELF/Mach-O section */
extern const char build_metadata[];

/* Helper function to print detailed version info */
void print_version_info(void);

#endif /* BUILDINFO_H */
