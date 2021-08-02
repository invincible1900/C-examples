// man lspci
// man iopl

#include <sys/io.h>
#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>

#define CONFIG_ADDR 0xCF8
#define CONFIG_DATA 0xCFC

#define VENDOR_ID 0x8086
#define EHCI_PRODUCT_ID 0x293A

#define PCI_BASE_ADDR 0x80000000L
#define PCI_MAX_BUS 255
#define PCI_MAX_DEV 31
#define PCI_MAX_FUN 7

#define HEADER_TYPE 0x0c
#define BAR_0 0x10

void do_lspci(int bus, int dev, int fun){
        char tmp_cmd[100];
        memset(tmp_cmd, 0, sizeof(tmp_cmd));
        sprintf(tmp_cmd, "lspci | grep %02d:%02d.%x", bus, dev, fun);
        system(tmp_cmd);
}


int get_config(void)
{
        int bus, dev, fun, addr, data;
        for (bus = 0; bus <= PCI_MAX_BUS; bus++)
        {
                for (dev = 0; dev <= PCI_MAX_DEV; dev++)
                {
                        for (fun = 0; fun <= PCI_MAX_FUN; fun++)
                        {
                                addr = PCI_BASE_ADDR | (bus << 16) | (dev << 11) | (fun << 8);
                                outl(addr, CONFIG_ADDR);
                                data = inl(CONFIG_DATA);
                                if((data & 0xffff) == 0xffff )
                                        continue;
                                printf("addr: %x(bus: %x, dev: %x, func: %x), VENDOR_ID: %x, DEVICE_ID: %x\n",
                                                addr, bus, dev, fun, data & 0xffff, data >> 16);

                                do_lspci(bus, dev, fun);

                                outl(addr + HEADER_TYPE, CONFIG_ADDR);
                                data = inl(CONFIG_DATA);
				printf("%x\n", data);
                                data = (data >> 16) & 0xff;
                                if(data == 0x01){
                                        printf("PCI-to-PCI bridge\n");
                                }else{
                                        printf("PCI device\n");
                                }

                                for(int i = 0; i < 6; i++){
                                        outl(addr + BAR_0 + i*4, CONFIG_ADDR);
                                        data = inl(CONFIG_DATA);

                                        if(data & 0x1){
                                                printf("BAR_%d: %x(ioport), base addr: %x\n",
                                                                i, data, data & 0xfffffffc);
                                        }else{
                                                printf("BAR_%d: %x(iomem), base addr: %x\n",
                                                                i, data, data & 0xfffffff0);
                                        }


                                }
                                printf("\n");

                        }
                }
        }
        return -1;
}

int main(){
        iopl(3);
        get_config();
}


