/************************************************************
 * Desc: Open an elf file and read elf header ( no error check )
 *
 * Author: X++D
 *
 ***********************************************************/
#include <elf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

int read_elf_header(char *path) {

        Elf64_Ehdr ehdr;

        FILE *fd = fopen(path, "rb");

        fread(&ehdr, 1, sizeof(Elf64_Ehdr), fd);

        dump_elf_header(&ehdr);

        pclose(fd);

        return 0;
}

int main(int argc, char *argv[]){

        read_elf_header(argv[1]);

        return 0;
}

/*************************************************************************

$ ./elf /bin/ls
Magic:	7f 45 4c 46 02 01 01 00 00 00 00 00 00 00 00 00
Class: ELF64
Machine: AMD x86-64 architecture
Entry point address: 26576
Start of section headers: 140224
Size of section header: 64
Number of section headers: 30
Section header string table index: 29

$ readelf -h /bin/ls
ELF Header:
  Magic:   7f 45 4c 46 02 01 01 00 00 00 00 00 00 00 00 00
  Class:                             ELF64
  Data:                              2's complement, little endian
  Version:                           1 (current)
  OS/ABI:                            UNIX - System V
  ABI Version:                       0
  Type:                              DYN (Shared object file)
  Machine:                           Advanced Micro Devices X86-64
  Version:                           0x1
  Entry point address:               0x67d0
  Start of program headers:          64 (bytes into file)
  Start of section headers:          140224 (bytes into file)
  Flags:                             0x0
  Size of this header:               64 (bytes)
  Size of program headers:           56 (bytes)
  Number of program headers:         13
  Size of section headers:           64 (bytes)
  Number of section headers:         30
  Section header string table index: 29

#define EI_NIDENT (16)

typedef struct
{
  unsigned char e_ident[EI_NIDENT];     // magic number and other info 
  Elf64_Half    e_type;                 // object file type 
  Elf64_Half    e_machine;              // architecture 
  Elf64_Word    e_version;              // object file version 
  Elf64_Addr    e_entry;                // entry point virtual address 
  Elf64_Off     e_phoff;                // program header table file offset 
  Elf64_Off     e_shoff;                // section header table file offset 
  Elf64_Word    e_flags;                // processor-specific flags 
  Elf64_Half    e_ehsize;               // elf header size in bytes 
  Elf64_Half    e_phentsize;            // program header table entry size 
  Elf64_Half    e_phnum;                // program header table entry count 
  Elf64_Half    e_shentsize;            // section header table entry size 
  Elf64_Half    e_shnum;                // section header table entry count 
  Elf64_Half    e_shstrndx;             // section header string table index 
} Elf64_Ehdr; 

#define EM_X86_64       62      // AMD x86-64 architecture 
#define EM_386           3      // Intel 80386 


*************************************************************************/
