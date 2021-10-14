/*
 * Creating a KVM-based VM basically needs 7 steps: 
 *
 * 1. Open the KVM device, `kvmfd=open("/dev/kvm", O_RDWR|O_CLOEXEC)`
 * 2. Do create a VM, `vmfd=ioctl(kvmfd, KVM_CREATE_VM, 0)`
 * 3. Set up memory for VM guest, `ioctl(vmfd, KVM_SET_USER_MEMORY_REGION, &region)`
 * 4. Create a virtual CPU for the VM, `vcpufd=ioctl(vmfd, KVM_CREATE_VCPU, 0)`
 * 5. Set up memory for the vCPU
 *     - `vcpu_size=ioctl(kvmfd, KVM_GET_VCPU_MMAP_SIZE, NULL)`
 *     - `run=(struct kvm_run*)mmap(NULL, mmap_size, PROT_READ|PROT_WRITE, MAP_SHARED, vcpufd, 0)`
 * 6. Put assembled code on user memory region, set up vCPU's registers such as rip
 * 7. Run and handle exit reason. `while(1) { ioctl(vcpufd, KVM_RUN, 0); ... }`
 * 
 * https://david942j.blogspot.com/2018/10/note-learning-kvm-implement-your-own.html
 */

#include <stdio.h>
#include <linux/kvm.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

#define NONE "\033[m"
#define LIGHT_BLUE "\033[1;34m"
#define DBG_MSG(fmt, ...) \
        while(1) { \
                printf("[D] %s:%d, ", __func__, __LINE__); \
                printf(LIGHT_BLUE fmt NONE, ##__VA_ARGS__); \
                break; \
        }


struct vm {
	int dev_fd; 	// open "/dev/kvm"
	int vm_fd;  	// KVM_CREATE_VM
	int vcpu_fd;	// KVM_CREATE_VCPU

	__u64 ram_start;      // KVM_SET_USER_MEMORY_REGION
	__u64 ram_size; // KVM_SET_USER_MEMORY_REGION

	struct kvm_run *run; // mmap KVM_GET_VCPU_MMAP_SIZE
};

/* 加载 vm binary */
void load_binary(struct vm *vm, char *path){

	int fd = open(path, O_RDONLY);
	int ret = 0;
	char *p = (char *)vm->ram_start;
	
	while(1){
		ret = read(fd, p, 4096);
		if(ret <= 0)
			break;
		// DBG_MSG("read size: %d\n", ret);
		p += ret;	
	}
}

void run_vm(struct vm *vm){
	DBG_MSG("%s\n", "KVM start run");
	while(1){

		/* KVM_RUN */
		ioctl(vm->vcpu_fd, KVM_RUN, 0);
		switch(vm->run->exit_reason){
			case KVM_EXIT_UNKNOWN:
				DBG_MSG("KVM_EXIT_UNKNOWN\n");
				break;
			case KVM_EXIT_IO:
				DBG_MSG("KVM_EXIT_IO, out port: 0x%x, data: %d\n",
                                		vm->run->io.port,
                                		*(int *)((void *)(vm->run) + vm->run->io.data_offset)
					);
				break;
	                case KVM_EXIT_MMIO:
	                        DBG_MSG("KVM_EXIT_MMIO\n");
	                        break;
	                case KVM_EXIT_HLT:
	                        DBG_MSG("KVM_EXIT_HLT\n");
	                        goto exit_kvm;
	                case KVM_EXIT_SHUTDOWN:
	                        DBG_MSG("KVM_EXIT_SHUTDOWN\n");
	                        goto exit_kvm;
	                default:
	                        DBG_MSG("KVM EXIT: %d\n", vm->run->exit_reason);
	                        goto exit_kvm;


		}
	}
exit_kvm:
	return;
}

int main(int argc, char *argv[]){	
	int ret;	
	struct vm *vm = malloc(sizeof(struct vm));

	/* KVM_CREATE_VM  */
	vm->dev_fd = open("/dev/kvm", O_RDWR);
	DBG_MSG("vm->dev_fd: %d\n", vm->dev_fd);

	vm->vm_fd = ioctl(vm->dev_fd, KVM_CREATE_VM, 0);
	DBG_MSG("vm->vm_fd: %d\n", vm->vm_fd);

	vm->ram_size = 0x200000;
	vm->ram_start= (__u64)mmap(NULL, vm->ram_size, PROT_READ | PROT_WRITE, 
				MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE, -1, 0);
	DBG_MSG("vm->ram_start: %p\n", (void *)vm->ram_start);

	/* KVM_SET_USER_MEMORY_REGION */
	struct kvm_userspace_memory_region mem;
	mem.slot = 0;
	mem.guest_phys_addr = 0;
	mem.memory_size = vm->ram_size;
	mem.userspace_addr = vm->ram_start;
	ret =ioctl(vm->vm_fd, KVM_SET_USER_MEMORY_REGION, &mem);
	DBG_MSG("KVM_SET_USER_MEMORY_REGION ret: %d\n", ret);

	/* KVM_CREATE_VCPU */	
	int run_mmap_size;
	vm->vcpu_fd = ioctl(vm->vm_fd, KVM_CREATE_VCPU, 0);
	DBG_MSG("vm->vcpu_fd: %d\n", vm->vcpu_fd);

	/* KVM_GET_VCPU_MMAP_SIZE */
	run_mmap_size = ioctl(vm->dev_fd, KVM_GET_VCPU_MMAP_SIZE, 0);
	vm->run = mmap(NULL, run_mmap_size, PROT_READ | PROT_WRITE, 
				MAP_SHARED, vm->vcpu_fd, 0);
	DBG_MSG("vm->run: %p\n", vm->run);

	/* KMV_SET_SREGS */
	struct kvm_sregs sregs;
	ioctl(vm->vcpu_fd, KVM_GET_SREGS, &sregs);
	sregs.cs.base = sregs.cs.selector = 0;
	ioctl(vm->vcpu_fd, KVM_SET_SREGS, &sregs);

	/* KMV_SET_REGS */
	struct kvm_regs  regs;
        ioctl(vm->vcpu_fd, KVM_SET_REGS, &(regs));
        regs.rflags = 0x2ULL;
        regs.rip = 0;
        regs.rsp = 0x1000;
        regs.rbp= 0;
        ret = ioctl(vm->vcpu_fd, KVM_SET_REGS, &(regs));
	DBG_MSG("KVM_SET_REGS ret: %d\n", ret);


	/* Load binary */	
	if(argc < 2)
		load_binary(vm, "a.bin");
	else
		load_binary(vm, argv[1]);


	run_vm(vm);
		
}

