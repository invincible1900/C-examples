### How to

```
make
./mmu
./read_mem [mmu pid] [string virtual address]
```

```
$ ./mmu
[D] pid: 808305
Where am I?
[D] addr: 0x5555671666b0
[D] offset: 183252519728
[D] pme: 0x8180000000325045
[D] gfn: 0x325045
Your physical address is at 0x3250456b0

$ ./v2p 808305 0x5555671666b0
0x3250456b0

$ ./read_mem 808305 0x5555671666b0
path: /proc/808305/mem, offset: 93825290102448
57 68 65 72 65 20 61 6D  20 49 3F 00 00 00 00 00  |  Where am I?.....
...

```


```
read_phy_mm.c

man mem

DESCRIPTION
       /dev/mem  is  a  character device file that is an image of the main memory of the computer. 
...
       Byte addresses in /dev/mem are interpreted as physical memory addresses.  
...
       Since Linux 2.6.26, and depending on the architecture, the CONFIG_STRICT_DEVMEM kernel configuration  option
       limits  the  areas  which can be accessed through this file.  For example: on x86, RAM access is not allowed
       but accessing memory-mapped PCI regions is.

```
