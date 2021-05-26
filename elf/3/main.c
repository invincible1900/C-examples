/************************************************************
 * Desc: mmap an elf file and read elf section headers ( no error check )
 *
 * Author: X++D
 *
 ***********************************************************/

#include <elf.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

void read_elf_section_headers(Elf64_Ehdr *ehdr){

        Elf64_Shdr *sechdrs;
        Elf64_Shdr *s, *se;
        char *secstrs;

        size_t sh_size;
        loff_t sh_off;
        int i;


        sh_size = ehdr->e_shnum * sizeof(Elf64_Shdr);

        sh_off = ehdr->e_shoff;

        sechdrs = (Elf64_Shdr *)((char *)ehdr + sh_off);

        secstrs = (char *)ehdr + sechdrs[ehdr->e_shstrndx].sh_offset;

        for (i = 0, s = sechdrs, se = sechdrs + ehdr->e_shnum; s < se; s++, i++) {
                printf("********************************************\n");
                printf("Section name: %s [%d]\n", secstrs + s->sh_name, i);
                printf("section offset: %lu\n", s->sh_offset);
                printf("section size: %lu\n", s->sh_size);
                printf("********************************************\n");
        }

}


int main(int argc, char *argv[]){
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

        fd = open(path, O_RDONLY);

        fstat(fd, &stat);

        file_size = stat.st_size;

        maddr = mmap(NULL, file_size, PROT_READ, MAP_FILE | MAP_PRIVATE, fd, 0);

        read_elf_section_headers((Elf64_Ehdr *)maddr);

        munmap(maddr, file_size);

        return 0;
}


/*************************************************************************
$ ./elf
...
********************************************
Section name: .text [16]
section offset: 19872
section size: 75730
********************************************
...
********************************************
Section name: .data [26]
section offset: 139264
section size: 616
********************************************
********************************************
Section name: .bss [27]
section offset: 139880
section size: 4824
********************************************
...
********************************************
Section name: .shstrtab [29]
section offset: 139932
section size: 285
********************************************

$ readelf -S /bin/ls

Section Headers:
  [Nr] Name              Type             Address           Offset
       Size              EntSize          Flags  Link  Info  Align
...

  [16] .text             PROGBITS         0000000000004da0  00004da0
       00000000000127d2  0000000000000000  AX       0     0     16
...

  [26] .data             PROGBITS         0000000000023000  00022000
       0000000000000268  0000000000000000  WA       0     0     32
  [27] .bss              NOBITS           0000000000023280  00022268
       00000000000012d8  0000000000000000  WA       0     0     32

...

  [29] .shstrtab         STRTAB           0000000000000000  0002229c
       000000000000011d  0000000000000000           0     0     1

*************************************************************************/








