#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <signal.h>
#include <syscall.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/reg.h>
#include <sys/user.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include <fcntl.h>

#include "elf64.h"

#define	ET_NONE	0	//No file type 
#define	ET_REL	1	//Relocatable file 
#define	ET_EXEC	2	//Executable file 
#define	ET_DYN	3	//Shared object file 
#define	ET_CORE	4	//Core file 

void find_section_header(Elf64_Shdr* my_elf_section_header_temp, int fd_of_elf_file, int offset_of_section_table, int size_of_section_header,
                                int number_of_sections, int offset_of_strings, int size_of_strings, int cant_be_index,
                                char* section_name){
    int i = 1;
    while (i < number_of_sections) {
        int size_of_header_temp = pread(fd_of_elf_file, my_elf_section_header_temp, sizeof(*my_elf_section_header_temp),
                                        offset_of_section_table + (i * size_of_section_header));
        int offset_by_name = my_elf_section_header_temp->sh_name;
        int offset_index = offset_of_strings + offset_by_name;
        while (offset_index < (offset_of_strings + size_of_strings)) {
            char* find_sym = (char *) malloc(size_of_strings);
            int next = pread(fd_of_elf_file, find_sym, size_of_strings, offset_index);
            if (!strcmp(find_sym, section_name)) {
                free(find_sym);
                break;
            }
            free(find_sym);
            offset_index = offset_index + next + 1;
        }
        if (offset_index < (offset_of_strings + size_of_strings)) {
            break;
        }
        i += 1;
    }
}

bool find_symbol_entry(Elf64_Sym* my_elf_symbol_entry, int fd_of_elf_file, int offset_of_section_table, int size_of_section_header,
                         int number_of_sections, int offset_of_strings, int size_of_strings ,bool* is_local, char* section_name){
    int i = 1;
    bool is_exist = false;
    while (i < number_of_sections) {
        int size_of_header_temp = pread(fd_of_elf_file, my_elf_symbol_entry, sizeof(*my_elf_symbol_entry),
                                        offset_of_section_table + (i * size_of_section_header));
        int offset_by_name = my_elf_symbol_entry->st_name;
        int offset_index = offset_of_strings + offset_by_name;
        while (offset_index < (offset_of_strings + size_of_strings)) {
            char* find_sym = (char *) malloc(size_of_strings);
            int next = pread(fd_of_elf_file, find_sym, size_of_strings, offset_index);
            if (!strcmp(find_sym, section_name)) {
                is_exist = true;
                if (ELF64_ST_BIND(my_elf_symbol_entry->st_info) == 0x0){
                    *is_local = true;
                }
                else{
                    if (ELF64_ST_BIND(my_elf_symbol_entry->st_info) == 0x1){
                        *is_local = false;
                        free(find_sym);
                        break;
                    }
                }
            }
            free(find_sym);
            offset_index = offset_index + next + 1;
        }
        if (offset_index < (offset_of_strings + size_of_strings)) {
            break;
        }
        i += 1;
    }
    return is_exist;
}

/* symbol_name		- The symbol (maybe function) we need to search for.
 * exe_file_name	- The file where we search the symbol in.
 * error_val		- If  1: A global symbol was found, and defined in the given executable.
 * 			- If -1: Symbol not found.
 *			- If -2: Only a local symbol was found.
 * 			- If -3: File is not an executable.
 * 			- If -4: The symbol was found, it is global, but it is not defined in the executable.
 * return value		- The address which the symbol_name will be loaded to, if the symbol was found and is global.
 */
unsigned long find_symbol(char* symbol_name, char* exe_file_name, int* error_val) {
    int fd_of_elf_file = open(exe_file_name, O_RDONLY);  //add if it's -1
    Elf64_Ehdr my_elf_header;
    int size_of_elf_header = read(fd_of_elf_file, &my_elf_header, sizeof(my_elf_header));
    if (my_elf_header.e_type != ET_EXEC){
        *error_val = -3;
        return 0;
    }

    int index_of_strings = my_elf_header.e_shstrndx;
    int size_of_section_header = my_elf_header.e_shentsize;
    int offset_of_section_table = my_elf_header.e_shoff;
    Elf64_Shdr my_elf_section_header;
    int size_of_section_string = pread(fd_of_elf_file, &my_elf_section_header, sizeof(my_elf_section_header),
                                       offset_of_section_table + (index_of_strings * size_of_section_header));
    int offset_of_strings = my_elf_section_header.sh_offset;
    int size_of_strings = my_elf_section_header.sh_size;
    int number_of_sections = my_elf_header.e_shnum;

    Elf64_Shdr my_elf_symtab_header;
    find_section_header(&my_elf_symtab_header, fd_of_elf_file, offset_of_section_table, size_of_section_header, number_of_sections,
                                                          offset_of_strings, size_of_strings, index_of_strings, ".symtab");

    Elf64_Shdr my_elf_strtab_header;
    find_section_header(&my_elf_strtab_header, fd_of_elf_file, offset_of_section_table, size_of_section_header, number_of_sections,
                        offset_of_strings, size_of_strings, index_of_strings, ".strtab");

    int offset_of_symbol_table = my_elf_symtab_header.sh_offset;
    int size_of_symbol_table = my_elf_symtab_header.sh_size;
    int size_of_single_entry = my_elf_symtab_header.sh_entsize;
    int number_of_entries = size_of_symbol_table / size_of_single_entry;
    int size_of_strtab = my_elf_strtab_header.sh_size;
    int offset_of_strtab = my_elf_strtab_header.sh_offset;
    Elf64_Sym my_elf_symbol_entry;
    bool is_local_in_symtab = false;
    bool is_exist_in_symtab = find_symbol_entry(&my_elf_symbol_entry, fd_of_elf_file, offset_of_symbol_table, size_of_single_entry, number_of_entries,
                        offset_of_strtab, size_of_strtab, &is_local_in_symtab, symbol_name);
    if (!(is_exist_in_symtab)){
        *error_val = -1;
        return 0;
    }
    if (is_local_in_symtab){
        *error_val = -2;
        return 0;
    }
    if (my_elf_symbol_entry.st_shndx == SHN_UNDEF){
        *error_val = -4;
        return 0;
    }
    *error_val = 1;
	return my_elf_symbol_entry.st_value;
}

int main(int argc, char *const argv[]) {
	int err = 0;
	unsigned long addr = find_symbol(argv[1], argv[2], &err);

	if (err >= 0)
		printf("%s will be loaded to 0x%lx\n", argv[1], addr);
	else if (err == -2)
		printf("%s is not a global symbol! :(\n", argv[1]);
	else if (err == -1)
		printf("%s not found!\n", argv[1]);
	else if (err == -3)
		printf("%s not an executable! :(\n", argv[2]);
	else if (err == -4)
		printf("%s is a global symbol, but will come from a shared library\n", argv[1]);
	return 0;
}