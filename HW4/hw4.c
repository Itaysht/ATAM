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

bool find_sym_in_rela(Elf64_Rela* my_elf_rela_entry, int fd_of_elf_file, int offset_of_rela,
                      int size_of_rela_entry, int number_of_entries, Elf64_Shdr* my_elf_related_symtable,
                      Elf64_Shdr* my_elf_related_strtable, char* required_name){
    int i = 0;
    while (i < number_of_entries) {
        Elf64_Sym my_elf_related_symbol_entry;
        pread(fd_of_elf_file, my_elf_rela_entry, sizeof(*my_elf_rela_entry),
                                        offset_of_rela + (i * size_of_rela_entry));
        int index_in_related_symtable = ELF64_R_SYM(my_elf_rela_entry->r_info);
        pread(fd_of_elf_file, &my_elf_related_symbol_entry, sizeof(my_elf_related_symbol_entry),
                                        my_elf_related_symtable->sh_offset + (index_in_related_symtable * my_elf_related_symtable->sh_entsize));
        int offset_in_related_strtable = my_elf_related_symbol_entry.st_name;
        int offset_of_related_strtable = my_elf_related_strtable->sh_offset;

        char* find_sym = (char *) malloc(my_elf_related_strtable->sh_size);
        pread(fd_of_elf_file, find_sym, my_elf_related_strtable->sh_size, offset_of_related_strtable + offset_in_related_strtable);
        if (!strcmp(find_sym, required_name)) {
            free(find_sym);
            return true;
        }
        free(find_sym);
        i += 1;
    }
    return false;
}

unsigned long find_rela_section(int fd_of_elf_file, int offset_of_section_table, int size_of_section_header,
                      int number_of_sections, char* symbol_name) {
    int i = 0;
    while (i < number_of_sections) {
        Elf64_Shdr my_elf_section_header_temp;
        pread(fd_of_elf_file, &my_elf_section_header_temp, sizeof(my_elf_section_header_temp),
                                        offset_of_section_table + (i * size_of_section_header));
        if (my_elf_section_header_temp.sh_type == 0x4) //SHT_RELA
        {
            int index_of_symtable_related = my_elf_section_header_temp.sh_link;
            Elf64_Shdr my_elf_related_symtable;
            pread(fd_of_elf_file, &my_elf_related_symtable, sizeof(my_elf_related_symtable),
                  offset_of_section_table + (index_of_symtable_related * size_of_section_header));
            int index_of_strtable_related = my_elf_related_symtable.sh_link;
            Elf64_Shdr my_elf_related_strtable;
            pread(fd_of_elf_file, &my_elf_related_strtable, sizeof(my_elf_related_strtable),
                  offset_of_section_table + (index_of_strtable_related * size_of_section_header));

            int offset_of_rela = my_elf_section_header_temp.sh_offset;
            int size_of_rela_entry = my_elf_section_header_temp.sh_entsize;
            int number_of_entries = my_elf_section_header_temp.sh_size / size_of_rela_entry;

            Elf64_Rela my_elf_required_rela;
            bool something_found = find_sym_in_rela(&my_elf_required_rela, fd_of_elf_file, offset_of_rela, size_of_rela_entry,
                             number_of_entries, &my_elf_related_symtable, &my_elf_related_strtable, symbol_name);
            if (something_found)
            {
                return my_elf_required_rela.r_offset;
            }

        }
        i += 1;
    }
    return 0;
}

void find_section_header(Elf64_Shdr* my_elf_section_header_temp, int fd_of_elf_file, int offset_of_section_table, int size_of_section_header,
                                int number_of_sections, int offset_of_strings, int size_of_strings,
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
                                                          offset_of_strings, size_of_strings, ".symtab");

    Elf64_Shdr my_elf_strtab_header;
    find_section_header(&my_elf_strtab_header, fd_of_elf_file, offset_of_section_table, size_of_section_header, number_of_sections,
                        offset_of_strings, size_of_strings, ".strtab");

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
    *error_val = 1;
    if (my_elf_symbol_entry.st_shndx == SHN_UNDEF){
        *error_val = -4;
        return find_rela_section(fd_of_elf_file, offset_of_section_table, size_of_section_header,
                                 number_of_sections, symbol_name);
    }
	return my_elf_symbol_entry.st_value;
}

pid_t run_procces(const char* program){
    pid_t child;

    child = fork();
    if (child > 0){
        return child;
    }
    else{
        if (child == 0){
            if (ptrace(PTRACE_TRACEME, 0, NULL, NULL) < 0){
                perror("ptrace");
                return -1;
            }
            execl(program, program, NULL);
        }
        else{
            perror("fork");
            return -1;
        }
    }
}

void run_debugger(pid_t child_pid, unsigned long addr, bool dynamic_first_time){
    int counter = 1;
    unsigned char temp_decoder;
    int wait_status;
    struct user_regs_struct regs;
    wait(&wait_status);
    long data = ptrace(PTRACE_PEEKTEXT, child_pid, (void*)addr, NULL);

    long real_data = data;
    if (dynamic_first_time) {
        data = data - 6;
        real_data = ptrace(PTRACE_PEEKTEXT, child_pid, (void *) data, NULL);
    }

    while (WIFSTOPPED(wait_status)) {
        unsigned long data_trap = (real_data & 0xFFFFFFFFFFFFFF00) | 0xCC;
        if (dynamic_first_time){
            ptrace(PTRACE_POKETEXT, child_pid, (void *) data, (void *) data_trap);
        }
        else{
            ptrace(PTRACE_POKETEXT, child_pid, (void *) addr, (void *) data_trap);
        }
        ptrace(PTRACE_GETREGS, child_pid, 0, &regs);
        ptrace(PTRACE_CONT, child_pid, NULL, NULL);
        wait(&wait_status);

        ptrace(PTRACE_GETREGS, child_pid, 0, &regs);
        int requested_rdi = (int)regs.rdi;

        if (dynamic_first_time){
            ptrace(PTRACE_POKETEXT, child_pid, (void *) data, (void *) real_data);
        }
        else{
            ptrace(PTRACE_POKETEXT, child_pid, (void *) addr, (void *) real_data);
        }
        regs.rip -= 1;
        ptrace(PTRACE_SETREGS, child_pid, 0, &regs);
        ptrace(PTRACE_GETREGS, child_pid, 0, &regs);
        long val_of_rsp = ptrace(PTRACE_PEEKTEXT, child_pid, (void*)regs.rsp, NULL);
        int recur_counter = 1;

        temp_decoder = ptrace(PTRACE_PEEKTEXT, child_pid, (void*)regs.rip, 0);
        if (temp_decoder == 0xe8) {
            recur_counter++;
        }
        if (temp_decoder == 0xc3) {
            recur_counter--;
        }
        if (recur_counter == 0){
            printf("PRF:: run #%d first parameter is %d\n", counter, requested_rdi);
            printf("PRF:: run #%d returned with %d\n", counter, regs.rax);
            if ((dynamic_first_time) && (counter == 1)) {
                data = ptrace(PTRACE_PEEKTEXT, child_pid, (void *) addr, NULL);
                real_data = ptrace(PTRACE_PEEKTEXT, child_pid, (void *) data, NULL);
            }
            counter++;
            ptrace(PTRACE_SINGLESTEP, child_pid, 0, 0);
            wait(&wait_status);
            continue;
        }

        while (WIFSTOPPED(wait_status)) {
            ptrace(PTRACE_SINGLESTEP, child_pid, 0, 0);
            wait(&wait_status);
            ptrace(PTRACE_GETREGS, child_pid, 0, &regs);
            temp_decoder = ptrace(PTRACE_PEEKTEXT, child_pid, (void*)regs.rip, 0);
            if (temp_decoder == 0xe8) {
                recur_counter++;
            }
            if (temp_decoder == 0xc3) {
                recur_counter--;
            }
            if ((val_of_rsp == regs.rip) && (recur_counter == 0)){
                printf("PRF:: run #%d first parameter is %d\n", counter, requested_rdi);
                printf("PRF:: run #%d returned with %d\n", counter, regs.rax);
                if ((dynamic_first_time) && (counter == 1)) {
                    data = ptrace(PTRACE_PEEKTEXT, child_pid, (void *) addr, NULL);
                    real_data = ptrace(PTRACE_PEEKTEXT, child_pid, (void *) data, NULL);
                }
                break;
            }
        }
        counter++;
    }
    ptrace(PTRACE_CONT, child_pid, NULL, NULL);
    wait(&wait_status);
}

int main(int argc, char *const argv[]) {
	int err = 0;

    pid_t child_pid = run_procces(argv[2]);
    if (child_pid == -1){
        return 0;
    }


	unsigned long addr = find_symbol(argv[1], argv[2], &err);


	if (err >= 0)
        run_debugger(child_pid, addr, false);
	else if (err == -2)
		printf("PRF:: %s is not a global symbol!\n", argv[1]);
	else if (err == -1)
		printf("PRF:: %s not found! :(\n", argv[1]);
	else if (err == -3)
		printf("PRF:: %s not an executable!\n", argv[2]);
    else if (err == -4)
        run_debugger(child_pid, addr, true);
	return 0;
}