/* extract-buildinfo.c - Extract build metadata from binaries
 * 
 * Cross-platform tool to read .buildinfo section from ELF/Mach-O binaries
 * 
 * Usage: extract-buildinfo <binary>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#ifdef __APPLE__
#include <mach-o/loader.h>
#include <mach-o/fat.h>
#else
#include <elf.h>
#endif

int extract_elf_buildinfo(FILE *f) {
#ifdef __APPLE__
    (void)f; // Unused on macOS
#endif
#ifndef __APPLE__
    Elf64_Ehdr ehdr;
    rewind(f);
    
    if (fread(&ehdr, sizeof(ehdr), 1, f) != 1) {
        fprintf(stderr, "Failed to read ELF header\n");
        return 1;
    }
    
    // Verify ELF magic
    if (memcmp(ehdr.e_ident, ELFMAG, SELFMAG) != 0) {
        fprintf(stderr, "Not a valid ELF file\n");
        return 1;
    }
    
    // Read section headers
    fseek(f, ehdr.e_shoff, SEEK_SET);
    Elf64_Shdr *sections = malloc(ehdr.e_shnum * sizeof(Elf64_Shdr));
    if (!sections) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    if (fread(sections, sizeof(Elf64_Shdr), ehdr.e_shnum, f) != ehdr.e_shnum) {
        free(sections);
        fprintf(stderr, "Failed to read section headers\n");
        return 1;
    }
    
    // Read string table for section names
    Elf64_Shdr *shstrtab = &sections[ehdr.e_shstrndx];
    char *strtab = malloc(shstrtab->sh_size);
    if (!strtab) {
        free(sections);
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    fseek(f, shstrtab->sh_offset, SEEK_SET);
    if (fread(strtab, shstrtab->sh_size, 1, f) != 1) {
        free(strtab);
        free(sections);
        fprintf(stderr, "Failed to read string table\n");
        return 1;
    }
    
    // Find .buildinfo section
    for (int i = 0; i < ehdr.e_shnum; i++) {
        char *name = strtab + sections[i].sh_name;
        if (strcmp(name, ".buildinfo") == 0) {
            char *data = malloc(sections[i].sh_size + 1);
            if (!data) {
                free(strtab);
                free(sections);
                fprintf(stderr, "Memory allocation failed\n");
                return 1;
            }
            
            fseek(f, sections[i].sh_offset, SEEK_SET);
            if (fread(data, sections[i].sh_size, 1, f) != 1) {
                free(data);
                free(strtab);
                free(sections);
                fprintf(stderr, "Failed to read .buildinfo section\n");
                return 1;
            }
            
            data[sections[i].sh_size] = '\0';
            printf("%s", data);
            
            free(data);
            free(strtab);
            free(sections);
            return 0;
        }
    }
    
    free(strtab);
    free(sections);
    fprintf(stderr, "No .buildinfo section found in binary\n");
    return 1;
#else
    fprintf(stderr, "ELF format not supported on this platform\n");
    return 1;
#endif
}

int extract_macho_buildinfo(FILE *f) {
#ifdef __APPLE__
    struct mach_header_64 mh;
    rewind(f);
    
    if (fread(&mh, sizeof(mh), 1, f) != 1) {
        fprintf(stderr, "Failed to read Mach-O header\n");
        return 1;
    }
    
    // Verify Mach-O magic
    if (mh.magic != MH_MAGIC_64 && mh.magic != MH_CIGAM_64) {
        fprintf(stderr, "Not a valid 64-bit Mach-O file\n");
        return 1;
    }
    
    // Iterate through load commands
    for (uint32_t i = 0; i < mh.ncmds; i++) {
        struct load_command lc;
        long pos = ftell(f);
        
        if (fread(&lc, sizeof(lc), 1, f) != 1) {
            fprintf(stderr, "Failed to read load command\n");
            return 1;
        }
        
        if (lc.cmd == LC_SEGMENT_64) {
            fseek(f, pos, SEEK_SET);
            struct segment_command_64 seg;
            
            if (fread(&seg, sizeof(seg), 1, f) != 1) {
                fprintf(stderr, "Failed to read segment command\n");
                return 1;
            }
            
            // Check each section in this segment
            for (uint32_t j = 0; j < seg.nsects; j++) {
                struct section_64 sect;
                
                if (fread(&sect, sizeof(sect), 1, f) != 1) {
                    fprintf(stderr, "Failed to read section\n");
                    return 1;
                }
                
                if (strcmp(sect.sectname, "__buildinfo") == 0) {
                    char *data = malloc(sect.size + 1);
                    if (!data) {
                        fprintf(stderr, "Memory allocation failed\n");
                        return 1;
                    }

                    fseek(f, sect.offset, SEEK_SET);
                    
                    if (fread(data, sect.size, 1, f) != 1) {
                        free(data);
                        fprintf(stderr, "Failed to read __buildinfo section\n");
                        return 1;
                    }
                    
                    data[sect.size] = '\0';
                    printf("%s", data);
                    
                    free(data);
                    return 0;
                }
            }
        }
        
        fseek(f, pos + lc.cmdsize, SEEK_SET);
    }
    
    fprintf(stderr, "No __buildinfo section found in binary\n");
    return 1;
#else
    fprintf(stderr, "Mach-O format not supported on this platform\n");
    return 1;
#endif
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <binary>\n", argv[0]);
        fprintf(stderr, "\n");
        fprintf(stderr, "Extract build metadata from a binary compiled with buildinfo support.\n");
        return 1;
    }
    
    FILE *f = fopen(argv[1], "rb");
    if (!f) {
        perror("fopen");
        return 1;
    }
    
    // Detect file format by magic bytes
    uint32_t magic;
    if (fread(&magic, sizeof(magic), 1, f) != 1) {
        fprintf(stderr, "Failed to read file magic\n");
        fclose(f);
        return 1;
    }
    
#ifdef __APPLE__
    if (magic == MH_MAGIC_64 || magic == MH_CIGAM_64) {
        int result = extract_macho_buildinfo(f);
        fclose(f);
        return result;
    }
#else
    // ELF magic: 0x7f 'E' 'L' 'F' = 0x464c457f
    if (magic == 0x464c457f) {
        int result = extract_elf_buildinfo(f);
        fclose(f);
        return result;
    }
#endif
    
    fprintf(stderr, "Unknown or unsupported binary format\n");
    fclose(f);
    return 1;
}
