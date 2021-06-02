/*****************************************************
 *
 * Desc: 插入一个 section 到 ELF 文件 (草稿代码, 方便自己抄)
 *
 * Author: X++D
 *
*****************************************************/
#include <elf.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>
#include <assert.h>

#define NONE "\033[m"  
#define RED "\033[0;32;31m"  
#define LIGHT_BLUE "\033[1;34m"  
#define DARK_GRAY "\033[1;30m"  
#define YELLOW "\033[1;33m"  

#define x_err(fmt, ...) \
        while(1) { \
                printf("[D] %s:%d, ", __func__, __LINE__); \
                printf(RED fmt NONE, ##__VA_ARGS__); \
                break; \
        }

#define x_log(fmt, ...) \
        while(1) { \
                printf("[D] %s:%d, ", __func__, __LINE__); \
                printf(YELLOW fmt NONE, ##__VA_ARGS__); \
                break; \
        }


struct fix_block {
	const char *desc; /* 描述当前block, 方便调试，对功能无影响 */

	void *old;	  /* 指向当前 block 数据的指针 */
	void *new; 	  /* 指向当前 block 新数据的指针 */
	
	size_t old_off;   /* 当前 block 在原文件中的偏移 */
	size_t old_size;  /* 当前 block 在原文件中的大小 */

	size_t new_off;   /* 当前 block 在新文件中的偏移 */
	size_t new_size;  /* 当前 block 在新文件中的大小 */

	struct fix_block *next; /* 单链表指针 */

};

/**********************************************************************************
计算每个fix_block 的 new_off 字段:

	- 外层循环遍历每个待更新的 fix_block: fb_x, 顺便累计扩展的大小

	- 内层循环遍历每个fix_block: fb_y

	- 如果fb_y 位于 fb_x 之前，则fb_y 的大小变化要累加到 fb_x 的 new_off 中

**********************************************************************************/
size_t fixup_new_offset(struct fix_block *fb_head){
	size_t added = 0;
	struct fix_block *fb_x, *fb_y;
	for(fb_x = fb_head; fb_x; fb_x = fb_x->next){
		added += (fb_x->new_size - fb_x->old_size);
		for(fb_y = fb_head; fb_y; fb_y = fb_y->next){
			if(fb_x->old_off > fb_y->old_off){
				if(fb_y->new_size - fb_y->old_size == 0)
					continue;

				x_log(YELLOW"updating fb: %s, old offset: %lu\n"NONE, fb_x->desc, fb_x->new_off);
				x_log(YELLOW"compared fb: %s, old offset: %lu, size changed: %lu\n"NONE, fb_y->desc, fb_y->old_off, fb_y->new_size - fb_y->old_size);
				fb_x->new_off += (fb_y->new_size - fb_y->old_size);
				x_log(YELLOW"fb: %s updated, new offset: %lu\n\n"NONE, fb_x->desc, fb_x->new_off);
			}
		}
	}

 	return added;
} 


/**********************************************************************************
计算每个 program header 的 p_offset 和  p_filesz 字段:

	- 外层循环遍历每个待更新的 program header: phdr

	- 内层循环遍历每个fix_block: fb

	- 如果 phdr->p_offset 位于 fb->old_off + fb->old_size 之后，则 fb 的大小变化要累加到 p_offset 中
	
	- 如果 phdr->p_offset + phdr->p_filesz 位于 fb->old_off 之前则不需要修改

	- 如果不是以上两种情况则 program header 指向的 segment 应该为 fb

**********************************************************************************/
int fixup_phdrs(Elf64_Ehdr *ehdr, struct fix_block *fb_phdr, struct fix_block *fb_head){
	Elf64_Phdr *phdrs = (Elf64_Phdr *)fb_phdr->new;
	Elf64_Phdr *phdr = phdrs;

	for(int i =0; i < ehdr->e_phnum; i++, phdr = phdrs + i) {
		for(struct fix_block *fb = fb_head; fb; fb = fb->next){
			// fb 大小没变，不需要处理
			if(fb->new_size - fb->old_size == 0)
				continue;

			// 在大小发生变化的 fb 之前，不需要处理
			if(phdr->p_offset + phdr->p_filesz <= fb->old_off)
				continue;

			// 在大小发生变化的 fb 之后，则需要更新 offset
			if( phdr->p_offset >= fb->old_off+fb->old_size  ){
				x_log(YELLOW"updating phdr->p_offset: %lu(%d)\n"NONE, phdr->p_offset, i);
				x_log(YELLOW"compared to fb: %s, fb->old_off: %lu, size changed: %lu\n"NONE, 
						 fb->desc, fb->old_off, fb->new_size - fb->old_size);
				phdr->p_offset += (fb->new_size - fb->old_size);
				x_log(YELLOW"updated, phdr->p_offset: %lu\n\n"NONE, phdr->p_offset);
			}
			// 与大小发生变化的 fb 存在交集
			else{
				//  assert(phdr->p_offset == fb->old_off);
				//  assert(phdr->p_filesz <= fb->new_size);
				x_log(YELLOW"updating phdr->p_filesz: %lu (p_offset: %lu, %d)\n"NONE, phdr->p_filesz, phdr->p_offset, i);
				x_log(YELLOW"compared to fb: %s, fb->old_off: %lu, size changed: %lu\n"NONE, 
						 fb->desc, fb->old_off, fb->new_size - fb->old_size);
				phdr->p_filesz += (fb->new_size - fb->old_size);
				x_log(YELLOW"updated, phdr->p_filesz: %lu\n\n"NONE, phdr->p_filesz);
			}
		}
	}
	return 0;
}

/**********************************************************************************
计算每个 section header 的 sh_offset 和 sh_size 字段:

	- 外层循环遍历每个待更新的 section header: shdr

	- 内层循环遍历每个fix_block: fb

	- 如果 shdr->sh_offset 位于 fb->old_off + fb->old_size 之后，则 fb 的大小变化要累加到 sh_offset 中
	
	- 如果 shdr->sh_offset + shdr->sh_size 位于 fb->old_off 之前则不需要修改

	- 如果不是以上两种情况则 section header 指向的 section 应该为 fb, 比如 strtble

**********************************************************************************/
int fixup_shdrs(Elf64_Ehdr *ehdr, struct fix_block *fb_shdr, struct fix_block *fb_head, char *strtbl){
	Elf64_Shdr *shdrs = (Elf64_Shdr *)fb_shdr->new;
	Elf64_Shdr *shdr = shdrs;

	// 遍历每个待更新的 section header: shdr，注意跳过最后一个，也就是新添加的section
	for(int i =0; i < ehdr->e_shnum - 1; i++, shdr = shdrs + i) {
		size_t s_start, s_end;

		// 计算当前 section header 对应数据区的 range
		s_start = shdr->sh_offset;
		s_end = shdr->sh_offset + shdr->sh_size;

		// 遍历每个fix_block: fb
		for(struct fix_block *fb = fb_head; fb; fb = fb->next){
			// fb 大小没变，不需要处理
			if((fb->new_size - fb->old_size == 0)){
				continue;
			}

			// 在大小发生变化的 fb 之前，不需要处理
			if(s_end <= fb->old_off){
#if 0
				x_log(DARK_GRAY"shdr: %s sh_offset: %lu, s_start: %lu, s_end %lu\n"NONE, strtbl + shdr->sh_name, shdr->sh_offset, s_start, s_end);
				x_log(DARK_GRAY"compared to fb: %s, data start: %lu, data end: %lu, size changed: %lu\n"NONE, 
				        fb->desc, fb->old_off, fb->old_off + fb->old_size, fb->new_size - fb->old_size);
				x_log(DARK_GRAY"shdr: %s sh_offset not changed\n\n"NONE, strtbl + shdr->sh_name)
#endif
				continue;
			}

			// 在大小发生变化的 fb 之后，则需要更新 offset
			if( s_start >= fb->old_off+fb->old_size  ){
	
				x_log(YELLOW"updating shdr: %s sh_offset: %lu, s_start: %lu, s_end %lu\n"NONE, strtbl + shdr->sh_name, shdr->sh_offset, s_start, s_end);
				x_log(YELLOW"compared to fb: %s, old data start: %lu, old data end: %lu, size changed: %lu\n"NONE, 
						 fb->desc, fb->old_off, fb->old_off + fb->old_size, fb->new_size - fb->old_size);
				shdr->sh_offset += (fb->new_size - fb->old_size);
				x_log(YELLOW"shdr: %s sh_offset updated: %lu\n\n"NONE, strtbl + shdr->sh_name, shdr->sh_offset);
			}
			// 与大小发生变化的 fb 存在交集
			else{
				// 应该是包含关系: data range 包含 fb
				//  assert(s_end >= fb->old_off + fb->old_size);
				x_log(YELLOW"updating shdr: %s sh_size: %lu, s_start: %lu, s_end %lu\n"NONE, 
					strtbl + shdr->sh_name, shdr->sh_size, s_start, s_end);
				x_log(YELLOW"compared to fb: %s, data start: %lu, data end: %lu, size changed: %lu\n"NONE, 
						 fb->desc, fb->old_off, fb->old_off + fb->old_size, fb->new_size - fb->old_size);
				if(strcmp(".bss", strtbl + shdr->sh_name) != 0){
					shdr->sh_size += (fb->new_size - fb->old_size);
				}else{
					// I don't know how to handle this.
					// It's strange that .bss offset + size is larger than the file size
					// Adding .bss size may cause crash after objcopy --remove-section
				}
				x_log(YELLOW"shdr: %s sh_size updated: %lu\n\n"NONE, strtbl + shdr->sh_name, shdr->sh_size);
				
			}
		}
	}
	
	return 0;
}

/********************************************************************
 * dump_fix_block: 打印一个初始化过的 fix_block, 用于 Debug
 *******************************************************************/
void dump_fix_block(struct fix_block *fb){
	printf("\nfb: %s\n", fb->desc);
	printf("\tfb->old: %p\n", fb->old);
	printf("\tfb->old_off: %lu\n", fb->old_off);
	printf("\tfb->old_size: %lu\n", fb->old_size);
	printf("\tfb->new: %p\n", fb->new);
	printf("\tfb->new_off: %lu\n", fb->new_off);
	printf("\tfb->new_size: %lu\n", fb->new_size);
	
	if(fb->next){
		printf("\tfb->next: %p, %s\n", fb->next, fb->next->desc);
	}else{
		printf("\tfb->next: NULL\n");
	}

	if(fb->new_size - fb->old_size != 0){
		printf(YELLOW"\tsize changed: %lu\n"NONE, fb->new_size - fb->old_size);
	}

	if(fb->new_off - fb->old_off != 0){
		printf(YELLOW"\toffset changed: %lu\n"NONE, fb->new_off - fb->old_off);
	}
	printf("\n");
}
	
/********************************************************************
 * dump_fix_blocks: 打印 fix_block 链表, 用于 Debug
 *******************************************************************/
void dump_fix_blocks(struct fix_block *fb_head){
	for(struct fix_block *fb = fb_head; fb; fb = fb->next){
		dump_fix_block(fb);
	}
}

int mmap_file(char *path, void **maddr, size_t *file_size, int *fd_o){
	int fd;
        struct stat stat;
	size_t f_sz;
        void *maddr_l;

        fd = open(path, O_RDONLY);
	if(fd < 0){
		x_err("open failed\n");
		return -1;
	}

	fstat(fd, &stat);
        f_sz = stat.st_size;

        maddr_l = mmap(NULL, f_sz, PROT_READ, MAP_FILE | MAP_PRIVATE, fd, 0);

	if(!maddr_l){
		x_err("mmap failed\n");
		return -1;
	}	

	*maddr = maddr_l;
	*file_size = f_sz;
	*fd_o = fd;

	return 0;
}

void unmap_file(int fd, void *maddr, size_t file_size){
	munmap(maddr, file_size);
	close(fd);
}

int elf_sanity_check(Elf64_Ehdr *elf_hdr){
        if (memcmp(elf_hdr->e_ident, ELFMAG, SELFMAG) != 0) {
		x_err("elf_sanity_check error\n");
                return -1;
        }

        if (!elf_hdr->e_shoff) {
		x_err("elf_sanity_check error\n");
                return -1;
        }

        if (elf_hdr->e_shentsize != sizeof(Elf64_Shdr)) {
		x_err("elf_sanity_check error\n");
                return -1;
        }

        if (elf_hdr->e_shnum > 65536U / sizeof(Elf64_Shdr)) {
		x_err("elf_sanity_check error\n");
                return -1;
        }

        if(elf_hdr->e_ident[EI_CLASS] != ELFCLASS64){
		x_err("elf_sanity_check error\n");
                return -1;
        }

	return 0;
}

int check_section_exists(Elf64_Ehdr *ehdr, Elf64_Shdr *sechdrs, char *secstrs, char *s_name){
        Elf64_Shdr *s, *se;
	
        for (s = sechdrs, se = sechdrs + ehdr->e_shnum; s < se; s++) {
                if (strcmp(s_name, secstrs + s->sh_name) == 0){
			return -1;
                }
        }
	return 0;
}

int insert_section(char *file_path, char *new_sec_name, void *new_sec, size_t new_sec_size){
	void *maddr;
	int fd;
	size_t file_size;

	/* 0. mmap 目标文件 */
	printf(LIGHT_BLUE"************************************************\n"NONE);
	printf(LIGHT_BLUE"0. mmap 目标文件\n"NONE);
	printf(LIGHT_BLUE"************************************************\n"NONE);
	if(mmap_file(file_path, &maddr, &file_size, &fd) < 0){
		x_err("mmap_file error\n");
		return -1;
	}
       	x_log("maddr: %p, file_size: %lu\n", maddr, file_size); 

	/* 0. 读取目标文件信息，并进行检查 */
	Elf64_Ehdr *ehdr = (Elf64_Ehdr *) maddr;
	if(elf_sanity_check(ehdr) < 0){
		unmap_file(fd, maddr, file_size);
		return -1;
	}
	Elf64_Shdr *shdrs = (Elf64_Shdr *)( maddr + ehdr->e_shoff);
	Elf64_Phdr *phdrs = (Elf64_Phdr *)( maddr + ehdr->e_phoff);
	char *strtbl =  maddr + (shdrs + ehdr->e_shstrndx)->sh_offset;
	size_t strtbl_size = (shdrs + ehdr->e_shstrndx)->sh_size;

	if(check_section_exists(ehdr, shdrs, strtbl, new_sec_name) < 0){
		x_err("section %s exists, remove it and try again\n", new_sec_name);
		x_err("objcopy --remove-section %s %s\n", new_sec_name, file_path);
		unmap_file(fd, maddr, file_size);
		return -1;
	}


	struct fix_block fb_ehdr;
	struct fix_block fb_shdr;
	struct fix_block fb_phdr;
	struct fix_block fb_strtbl;
	struct fix_block fb_sig;
	struct fix_block *fb = NULL, *fb_head = NULL;

	Elf64_Ehdr *new_ehdr;	
	size_t new_file_size;

	memset(&fb_ehdr, 0, sizeof(struct fix_block));
	memset(&fb_shdr, 0, sizeof(struct fix_block));
	memset(&fb_phdr, 0, sizeof(struct fix_block));
	memset(&fb_strtbl, 0, sizeof(struct fix_block));
	memset(&fb_sig, 0, sizeof(struct fix_block));

	

	/* 1. 初始化将要发生变化的几个 fix_block */ 
	printf(LIGHT_BLUE"************************************************\n"NONE);
	printf(LIGHT_BLUE"1. 初始化修改所需的信息\n"NONE);
	printf(LIGHT_BLUE"************************************************\n"NONE);
	fb = &fb_strtbl;
	fb->desc = "String table section";
	fb->old = strtbl; 
	fb->old_off = (shdrs + ehdr->e_shstrndx)->sh_offset;
	fb->old_size = strtbl_size;
        fb->new_size = strtbl_size + strlen(new_sec_name) + 1;
        fb->new = (char *)malloc(fb->new_size);
        memset(fb->new, 0, fb->new_size);
        memcpy(fb->new, fb->old, fb->old_size);
        memcpy(fb->new + fb->old_size, new_sec_name, strlen(new_sec_name));
        fb->new_off = fb->old_off; 
        fb->next = fb_head;
        fb_head = fb;

	/************************************************/
	fb = &fb_sig;
        fb->desc = "New section";
	fb->old = NULL;
	fb->old_size = 0;
	fb->old_off =  file_size;
        fb->new = new_sec;
        fb->new_size = new_sec_size;
        fb->new_off = fb->old_off;
        fb->next = fb_head;
        fb_head = fb;

	/************************************************/
	fb = &fb_shdr;
	fb->desc = "Section Headers";
	fb->old = (char *)shdrs;
	fb->old_off = ehdr->e_shoff;
	fb->old_size = ehdr->e_shentsize * ehdr->e_shnum;
	fb->new_size = fb->old_size + ehdr->e_shentsize;
	// 填充新的section header
	fb->new = malloc(fb->new_size);
	memset(fb->new, 0, fb->new_size);
	memcpy(fb->new, fb->old, fb->old_size);
	Elf64_Shdr *shdr_new = (Elf64_Shdr *)(fb->new + fb->old_size);
	shdr_new->sh_name = strtbl_size;
	shdr_new->sh_type = 0x80aabbcc;
	shdr_new->sh_flags = SHF_OS_NONCONFORMING;
	shdr_new->sh_offset = fb_sig.old_off;
	shdr_new->sh_size = new_sec_size;
	shdr_new->sh_addralign = 1;

        fb->new_off = fb->old_off;
        fb->next = fb_head;
        fb_head = fb;

	/************************************************/
	fb = &fb_phdr;
	fb->desc = "Program Headers";
	fb->old = (char *)phdrs;
	fb->old_size = ehdr->e_phentsize * ehdr->e_phnum;
	fb->old_off = ehdr->e_phoff;
	fb->new_size = fb->old_size;
	fb->new = malloc(fb->new_size);
	memcpy(fb->new, fb->old, fb->old_size);
        fb->new_off = fb->old_off;
	fb->next = fb_head;
        fb_head = fb;
	// dump_fix_block(fb);

	/************************************************/
	fb = &fb_ehdr;
	fb->desc = "ELF File Header";
	fb->old = (char *)ehdr;
	fb->old_size = ehdr->e_ehsize;
	fb->old_off = 0;
	fb->new_size = fb->old_size;
	fb->new = malloc(fb->new_size);
	memcpy(fb->new, fb->old, fb->old_size);
	new_ehdr = (Elf64_Ehdr *)fb->new;
	// new_ehdr->e_entry  不需要改
	// new_ehdr->e_phoff  需要修改
	// new_ehdr->e_shoff  需要修改
	new_ehdr->e_shnum = ehdr->e_shnum + 1;
	fb->new_off = fb->old_off;
	fb->next = fb_head;
        fb_head = fb;
	dump_fix_blocks(fb_head);

	// 2. 计算各个 fix_block 的 new_off 和 新的文件大小
	printf(LIGHT_BLUE"************************************************\n"NONE);
	printf(LIGHT_BLUE"2. 计算各个 fix_block 的 new_off 和 新的文件大小 \n"NONE);
	printf(LIGHT_BLUE"************************************************\n"NONE);
	new_file_size = fixup_new_offset(fb_head) +  file_size;
	x_log(YELLOW"new file size: %lu, old file size: %lu\n"NONE, new_file_size, file_size);
	dump_fix_blocks(fb_head);
	
	// 3. 修复新的 section headers 中 signature header 的偏移
	printf(LIGHT_BLUE"************************************************\n"NONE);
	printf(LIGHT_BLUE"3. 修复新的 section headers 中 signature header 的偏移\n"NONE);
	printf(LIGHT_BLUE"************************************************\n"NONE);
	x_log(YELLOW"shdr_new->sh_offset before: %lu\n"NONE, shdr_new->sh_offset);
        shdr_new->sh_offset = fb_sig.new_off;
	x_log(YELLOW"shdr_new->sh_offset after : %lu\n"NONE, shdr_new->sh_offset);

	// 4. 修复 ELF Header 中的偏移
	printf(LIGHT_BLUE"************************************************\n"NONE);
	printf(LIGHT_BLUE"4. 修复 ELF Header 中的偏移\n"NONE);
	printf(LIGHT_BLUE"************************************************\n"NONE);
	x_log(YELLOW"e_phoff before: %lu, e_shoff before: %lu\n"NONE, new_ehdr->e_phoff, new_ehdr->e_shoff);
	new_ehdr->e_phoff = fb_phdr.new_off;	
	new_ehdr->e_shoff = fb_shdr.new_off;	
	x_log(YELLOW"e_phoff after : %lu, e_shoff after : %lu\n"NONE, new_ehdr->e_phoff, new_ehdr->e_shoff);

	// 5. 修复 program headers 中每个 header 的偏移和大小
	printf(LIGHT_BLUE"************************************************\n"NONE);
	printf(LIGHT_BLUE"5. 修复 program headers 中每个 header 的偏移和大小\n"NONE);
	printf(LIGHT_BLUE"************************************************\n"NONE);
	fixup_phdrs(new_ehdr, &fb_phdr, fb_head);

	// 6. 修复 section headers 中每个 header 的偏移和大小
	printf(LIGHT_BLUE"************************************************\n"NONE);
	printf(LIGHT_BLUE"6. 修复 section headers 中每个 header 的偏移和大小\n"NONE);
	printf(LIGHT_BLUE"************************************************\n"NONE);
	fixup_shdrs(new_ehdr, &fb_shdr, fb_head, fb_strtbl.new);

	// 7. 开始 rewrite 整个文件
	printf(LIGHT_BLUE"************************************************\n"NONE);
	printf(LIGHT_BLUE"7. 开始 rewrite 整个文件\n"NONE);
	printf(LIGHT_BLUE"************************************************\n"NONE);
        char temp_path[20];
        strcpy(temp_path, "my.XXXXXX");
        int fd_new = mkstemp(temp_path);

	size_t f_pos_new = 0, f_pos_old = 0;

	// f_pos_new 指向写入的新文件当前位置，当它位于新文件末尾时循环结束
	while(f_pos_new < new_file_size){	
		size_t write_size = 0, ret = 0;
		struct fix_block *fb_next = NULL;

		// 先找到未写入的 new_off 最小的 fix_block，保存到 fb_next
		for(struct fix_block *fb = fb_head; fb; fb = fb->next){
			//  跳过已经写入的 block
			if(fb->new_off < f_pos_new){
				// x_log("Skip fb: %s, off: %lu, %lu\n", fb->desc, fb->new_off, f_pos_new);
				continue;
			}

			// 找到第一个待写入的 block
			if(fb_next == NULL){	
				fb_next = fb;
				// x_log("fb_next fb: %s, off: %lu, %lu\n", fb->desc, fb->new_off, f_pos_new);
				continue;
			}
	
			// 替换为更小的 block
			if(fb->new_off < fb_next->new_off){
				fb_next = fb;
				// x_log("fb_next update fb: %s, off: %lu, %lu\n", fb->desc, fb->new_off, f_pos_new);
			}
		}

#if 0
		if(fb_next){
			dump_fix_block(fb_next);
		}
#endif

		// 如果有待写入的 fix_block，且  new_off 与当前 f_pos_new 一致，则写入 fix_block->new 的内容
		if(fb_next && fb_next->new_off == f_pos_new){
			x_log(YELLOW"write fb: %s content, new file write positon: %lu, old file position: %lu\n"NONE, fb_next->desc, f_pos_new, f_pos_old);
			write_size = fb_next->new_size;
			x_log(YELLOW"content size: %lu\n"NONE, write_size);
			ret = write(fd_new, fb_next->new, write_size);
			x_log(YELLOW"wrote size: %lu\n"NONE, ret);
			if(ret != write_size){
				f_pos_new = -1;
				x_log(RED"write error\n"NONE);
				break;
			}else{
				f_pos_new += write_size;
				f_pos_old += fb_next->old_size;
			}
			x_log(YELLOW"upate file position: %lu(new), %lu(old)\n\n"NONE, f_pos_new, f_pos_old);
		}
		// 如果没有待写入的 fix_block，或者  new_off 与当前 f_pos_new 不一致，则写入原文件的内容
		else{
			x_log(YELLOW"write gap content, new file write positon: %lu, old file position: %lu\n"NONE, f_pos_new, f_pos_old);
			write_size = fb_next->new_off - f_pos_new;
			assert(write_size == fb_next->old_off - f_pos_old);
			x_log(YELLOW"content size: %lu\n"NONE, write_size);
			ret = write(fd_new,  maddr + f_pos_old, write_size);
			x_log(YELLOW"wrote size: %lu\n"NONE, ret);
			if(ret != write_size){
				f_pos_new = -1;
				x_log(RED"write error\n"NONE);
				break;
			}else{
				f_pos_new += write_size;
				f_pos_old += write_size;
			}
			x_log("upate file position: %lu(new), %lu(old)\n\n", f_pos_new, f_pos_old);
		}
	}

	close(fd_new);

	return 0;
}


int main(int argc, char *argv[]){
	char *path;
	if(argc < 2){
		path = "./ls";
	}else{
		path = argv[1];
	}
	size_t new_sec_size = 1024;
	void *new_sec = malloc(new_sec_size);
	memset(new_sec, 'x', new_sec_size);
	char *new_sec_name = ".X++D";
	
	insert_section(path, new_sec_name, new_sec, new_sec_size);

}
