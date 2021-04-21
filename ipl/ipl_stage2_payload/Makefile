PSPDEV=/usr/local/pspdev/bin
INCLUDES=-I $(PSPDEV)/../psp/sdk/include

all:	clean ipl_stage2_payload.bin

ipl_stage2_payload.bin: 
	$(PSPDEV)/psp-as crt0.s -o crt0.o
	$(PSPDEV)/psp-gcc -c main.c -o main.o -O2 -G0 -Wall -fno-pic $(CFLAGS)
	$(PSPDEV)/psp-as payloadex.s -o payloadex.o
	$(PSPDEV)/psp-ld -nostartfiles -T linkfile.l crt0.o payloadex.o main.o -o ipl_stage2_payload.elf
	$(PSPDEV)/psp-strip -s ipl_stage2_payload.elf
	$(PSPDEV)/psp-objcopy -O binary ipl_stage2_payload.elf ipl_stage2_payload.bin

clean:
	rm -rf *~ *.o *.elf *.bin

