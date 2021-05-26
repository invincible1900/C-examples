/************************************************************
 * Desc: mmap an elf file and read elf header ( no error check )
 *
 * Author: X++D
 *
 ***********************************************************/
#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <elf.h>

int dump_elf_header(Elf64_Ehdr *ehdr){
        printf("Magic:\t");
        for(int i = 0; i<EI_NIDENT; i++){
                printf("%02x ", ehdr->e_ident[i]);
        }
        printf("\n");

        if(ehdr->e_ident[EI_CLASS] == ELFCLASS64){
                printf("Class: %s\n", "ELF64");
        }

        if(ehdr->e_machine == EM_X86_64){
                printf("Machine: %s\n", "AMD x86-64 architecture");
        }

        printf("Entry point address: %lu\n", ehdr->e_entry);

        printf("Start of section headers: %lu\n", ehdr->e_shoff);
        printf("Size of section header: %u\n", ehdr->e_shentsize);
        printf("Number of section headers: %d\n", ehdr->e_shnum);
        printf("Section header string table index: %u\n", ehdr->e_shstrndx);

}

int main(int argc, char *argv[]){
        FILE *file;
        int fd;
        size_t file_size;
        struct stat stat;
        void *maddr;
        char *path;

        if(argc < 2){
                path = "/bin/ls";
        }else{
                path = argv[1];
        }
        file = fopen(path, "rb");

        fd = fileno(file);

        fstat(fd, &stat);

        file_size = stat.st_size;

        maddr = mmap(NULL, file_size, PROT_READ, MAP_FILE | MAP_PRIVATE, fd, 0);

        Elf64_Ehdr *ehdr = (Elf64_Ehdr *)maddr;

        dump_elf_header(ehdr);

        munmap(maddr, file_size);

        return 0;
}

/*************************************************************************

$ ./elf
Magic:	7f 45 4c 46 02 01 01 00 00 00 00 00 00 00 00 00
Class: ELF64
Machine: AMD x86-64 architecture
Entry point address: 26576
Start of section headers: 140224
Size of section header: 64
Number of section headers: 30
Section header string table index: 29

*************************************************************************/


