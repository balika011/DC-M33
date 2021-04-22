PSPDEV=$(shell psp-config --pspdev-path)

all: DC8 DC8/msipl.bin DC8/ipl_01g.bin DC8/ipl_02g.bin DC8/ipl_03g.bin DC8/nandipl_01g.bin DC8/nandipl_02g.bin DC8/nandipl_03g.bin DC8/kd/pspbtdnf.bin DC8/kd/pspbtjnf.bin DC8/kd/pspbtknf.bin DC8/kd/pspbtlnf.bin DC8/kd/pspbtrnf.bin DC8/kd/pspbtdnf_02g.bin DC8/kd/pspbtjnf_02g.bin DC8/kd/pspbtknf_02g.bin DC8/kd/pspbtlnf_02g.bin DC8/kd/pspbtrnf_02g.bin DC8/kd/pspbtdnf_03g.bin DC8/kd/pspbtjnf_03g.bin DC8/kd/pspbtknf_03g.bin DC8/kd/pspbtlnf_03g.bin DC8/kd/pspbtrnf_03g.bin DC8/tmctrl500.prx DC8/kd/ipl_update.prx DC8/kd/resurrection.prx DC8/kd/dcman.prx DC8/kd/iop.prx DC8/kd/lflash_fdisk.prx DC8/kd/idsregeneration.prx DC8/kd/emc_sm_updater.prx DC8/kd/lfatfs_updater.prx DC8/kd/lflash_fatfmt_updater.prx DC8/vsh/module/intraFont.prx DC8/vsh/module/vlf.prx DC8/kd/pspdecrypt.prx DC8/kd/galaxy.prx DC8/kd/idcanager.prx DC8/kd/march33.prx DC8/kd/popcorn.prx DC8/kd/systemctrl.prx DC8/kd/systemctrl_02g.prx DC8/kd/systemctrl_03g.prx DC8/kd/usbdevice.prx DC8/kd/vshctrl.prx DC8/vsh/module/recovery.prx DC8/vsh/module/satelite.prx

clean:
	rm -rf DC8
	make -C ipl/msiplldr clean
	make -C ipl/msipl clean
	make -C ipl/payloadex clean
	make -C ipl/ipl_stage2_payload clean
	rm -f ipl/ipl_stage2_payload/payloadex.s
	make -C ipl/ipl_stage1_payload clean
	rm -f ipl/ipl_stage1_payload/payload.s
	make -C ipl/nandipl clean
	make -C bootcnf clean
	make -C ipl/rebootex clean
	make -C modules/flashemu clean
	rm -f modules/flashemu/rebootex.S
	rm -f modules/flashemu/rebootex_02g.S
	rm -f modules/flashemu/rebootex_03g.S
	make -C modules/ipl_update clean
	make -C modules/vunbricker clean
	make -C modules/dcman clean
	make -C modules/iop clean
	make -C modules/lflash_fdisk clean
	make -C modules/idsregeneration clean

DC8:
	mkdir DC8
	cp -R 500/flash0/* DC8/
	cp -R prebuilt/* DC8/
	mkdir -p DC8/dic
	mkdir -p DC8/gps
	mkdir -p DC8/net/http
	mkdir -p DC8/registry
	mkdir -p DC8/vsh/theme

DC8/msipl.bin:
	make -C ipl/msiplldr
	psptools/pack_ipl.py ipl/msiplldr/msiplldr.bin DC8/msipl.bin

	make -C ipl/msipl
	dd if=ipl/msipl/msipl.bin of=DC8/msipl.bin bs=1 seek=4096

DC8/ipl_01g.bin:
	make -C ipl/payloadex clean
	make -C ipl/payloadex BFLAGS="-DIPL_01G -DMSIPL=1"
	$(PSPDEV)/bin/bin2s ipl/payloadex/payloadex.bin ipl/ipl_stage2_payload/payloadex.s payloadex

	make -C ipl/ipl_stage2_payload clean
	make -C ipl/ipl_stage2_payload CFLAGS="-DIPL_01G -DMSIPL=1"
	$(PSPDEV)/bin/bin2s ipl/ipl_stage2_payload/ipl_stage2_payload.bin ipl/ipl_stage1_payload/payload.s payload

	make -C ipl/ipl_stage1_payload clean
	make -C ipl/ipl_stage1_payload CFLAGS="-DIPL_01G -DMSIPL=1"

	dd if=ipl/ipl_stage1_payload/ipl_stage1_payload.bin of=DC8/ipl_01g.bin
	dd if=500/ipl_01g.bin of=DC8/ipl_01g.bin bs=1 seek=12288
	
DC8/ipl_02g.bin:
	make -C ipl/payloadex clean
	make -C ipl/payloadex BFLAGS="-DIPL_02G -DMSIPL=1"
	$(PSPDEV)/bin/bin2s ipl/payloadex/payloadex.bin ipl/ipl_stage2_payload/payloadex.s payloadex

	make -C ipl/ipl_stage2_payload clean
	make -C ipl/ipl_stage2_payload CFLAGS="-DIPL_02G -DMSIPL=1"
	$(PSPDEV)/bin/bin2s ipl/ipl_stage2_payload/ipl_stage2_payload.bin ipl/ipl_stage1_payload/payload.s payload

	make -C ipl/ipl_stage1_payload clean
	make -C ipl/ipl_stage1_payload CFLAGS="-DIPL_02G -DMSIPL=1"

	dd if=ipl/ipl_stage1_payload/ipl_stage1_payload.bin of=DC8/ipl_02g.bin
	dd if=500/ipl_02g.bin of=DC8/ipl_02g.bin bs=1 seek=12288
	
DC8/ipl_03g.bin:
	make -C ipl/payloadex clean
	make -C ipl/payloadex BFLAGS="-DIPL_03G -DMSIPL=1"
	$(PSPDEV)/bin/bin2s ipl/payloadex/payloadex.bin ipl/ipl_stage2_payload/payloadex.s payloadex

	make -C ipl/ipl_stage2_payload clean
	make -C ipl/ipl_stage2_payload CFLAGS="-DIPL_03G -DMSIPL=1"
	$(PSPDEV)/bin/bin2s ipl/ipl_stage2_payload/ipl_stage2_payload.bin ipl/ipl_stage1_payload/payload.s payload

	make -C ipl/ipl_stage1_payload clean
	make -C ipl/ipl_stage1_payload CFLAGS="-DIPL_03G -DMSIPL=1"

	dd if=ipl/ipl_stage1_payload/ipl_stage1_payload.bin of=DC8/ipl_03g.bin
	dd if=500/ipl_03g.bin of=DC8/ipl_03g.bin bs=1 seek=12288

ipl/nandipl/nandipl.bin:
	make -C ipl/nandipl


DC8/nandipl_01g.bin: ipl/nandipl/nandipl.bin
	psptools/pack_ipl.py ipl/nandipl/nandipl.bin DC8/nandipl_01g.bin

	make -C ipl/payloadex clean
	make -C ipl/payloadex BFLAGS="-DIPL_01G"
	$(PSPDEV)/bin/bin2s ipl/payloadex/payloadex.bin ipl/ipl_stage2_payload/payloadex.s payloadex

	make -C ipl/ipl_stage2_payload clean
	make -C ipl/ipl_stage2_payload CFLAGS="-DIPL_01G"
	$(PSPDEV)/bin/bin2s ipl/ipl_stage2_payload/ipl_stage2_payload.bin ipl/ipl_stage1_payload/payload.s payload

	make -C ipl/ipl_stage1_payload clean
	make -C ipl/ipl_stage1_payload CFLAGS="-DIPL_01G"

	dd if=ipl/ipl_stage1_payload/ipl_stage1_payload.bin of=DC8/nandipl_01g.bin bs=1 seek=4096
	dd if=500/ipl_01g.bin of=DC8/nandipl_01g.bin bs=1 seek=16384


DC8/nandipl_02g.bin: ipl/nandipl/nandipl.bin
	psptools/pack_ipl.py ipl/nandipl/nandipl.bin DC8/nandipl_02g.bin

	make -C ipl/payloadex clean
	make -C ipl/payloadex BFLAGS="-DIPL_02G"
	$(PSPDEV)/bin/bin2s ipl/payloadex/payloadex.bin ipl/ipl_stage2_payload/payloadex.s payloadex

	make -C ipl/ipl_stage2_payload clean
	make -C ipl/ipl_stage2_payload CFLAGS="-DIPL_02G"
	$(PSPDEV)/bin/bin2s ipl/ipl_stage2_payload/ipl_stage2_payload.bin ipl/ipl_stage1_payload/payload.s payload

	make -C ipl/ipl_stage1_payload clean
	make -C ipl/ipl_stage1_payload CFLAGS="-DIPL_02G"

	dd if=ipl/ipl_stage1_payload/ipl_stage1_payload.bin of=DC8/nandipl_02g.bin bs=1 seek=4096
	dd if=500/ipl_02g.bin of=DC8/nandipl_02g.bin bs=1 seek=16384


DC8/nandipl_03g.bin: ipl/nandipl/nandipl.bin
	psptools/pack_ipl.py ipl/nandipl/nandipl.bin DC8/nandipl_03g.bin

	make -C ipl/payloadex clean
	make -C ipl/payloadex BFLAGS="-DIPL_03G"
	$(PSPDEV)/bin/bin2s ipl/payloadex/payloadex.bin ipl/ipl_stage2_payload/payloadex.s payloadex

	make -C ipl/ipl_stage2_payload clean
	make -C ipl/ipl_stage2_payload CFLAGS="-DIPL_03G"
	$(PSPDEV)/bin/bin2s ipl/ipl_stage2_payload/ipl_stage2_payload.bin ipl/ipl_stage1_payload/payload.s payload

	make -C ipl/ipl_stage1_payload clean
	make -C ipl/ipl_stage1_payload CFLAGS="-DIPL_03G"

	dd if=ipl/ipl_stage1_payload/ipl_stage1_payload.bin of=DC8/nandipl_03g.bin bs=1 seek=4096
	dd if=500/ipl_03g.bin of=DC8/nandipl_03g.bin bs=1 seek=16384

bootcnf/bootcnf:
	make -C bootcnf

DC8/kd/pspbtdnf.bin: bootcnf/bootcnf cfg/pspbtdnf.txt
	./bootcnf/bootcnf -it cfg/pspbtdnf.txt -ob DC8/kd/pspbtdnf.bin

DC8/kd/pspbtjnf.bin: bootcnf/bootcnf cfg/pspbtjnf.txt
	./bootcnf/bootcnf -it cfg/pspbtjnf.txt -ob DC8/kd/pspbtjnf.bin

DC8/kd/pspbtknf.bin: bootcnf/bootcnf cfg/pspbtknf.txt
	./bootcnf/bootcnf -it cfg/pspbtknf.txt -ob DC8/kd/pspbtknf.bin

DC8/kd/pspbtlnf.bin: bootcnf/bootcnf cfg/pspbtlnf.txt
	./bootcnf/bootcnf -it cfg/pspbtlnf.txt -ob DC8/kd/pspbtlnf.bin

DC8/kd/pspbtrnf.bin: bootcnf/bootcnf cfg/pspbtrnf.txt
	./bootcnf/bootcnf -it cfg/pspbtrnf.txt -ob DC8/kd/pspbtrnf.bin

DC8/kd/pspbtdnf_02g.bin: bootcnf/bootcnf cfg/pspbtdnf_02g.txt
	./bootcnf/bootcnf -it cfg/pspbtdnf_02g.txt -ob DC8/kd/pspbtdnf_02g.bin

DC8/kd/pspbtjnf_02g.bin: bootcnf/bootcnf cfg/pspbtjnf_02g.txt
	./bootcnf/bootcnf -it cfg/pspbtjnf_02g.txt -ob DC8/kd/pspbtjnf_02g.bin

DC8/kd/pspbtknf_02g.bin: bootcnf/bootcnf cfg/pspbtknf_02g.txt
	./bootcnf/bootcnf -it cfg/pspbtknf_02g.txt -ob DC8/kd/pspbtknf_02g.bin

DC8/kd/pspbtlnf_02g.bin: bootcnf/bootcnf cfg/pspbtlnf_02g.txt
	./bootcnf/bootcnf -it cfg/pspbtlnf_02g.txt -ob DC8/kd/pspbtlnf_02g.bin

DC8/kd/pspbtrnf_02g.bin: bootcnf/bootcnf cfg/pspbtrnf_02g.txt
	./bootcnf/bootcnf -it cfg/pspbtrnf_02g.txt -ob DC8/kd/pspbtrnf_02g.bin

DC8/kd/pspbtdnf_03g.bin: bootcnf/bootcnf cfg/pspbtdnf_03g.txt
	./bootcnf/bootcnf -it cfg/pspbtdnf_03g.txt -ob DC8/kd/pspbtdnf_03g.bin

DC8/kd/pspbtjnf_03g.bin: bootcnf/bootcnf cfg/pspbtjnf_03g.txt
	./bootcnf/bootcnf -it cfg/pspbtjnf_03g.txt -ob DC8/kd/pspbtjnf_03g.bin

DC8/kd/pspbtknf_03g.bin: bootcnf/bootcnf cfg/pspbtknf_03g.txt
	./bootcnf/bootcnf -it cfg/pspbtknf_03g.txt -ob DC8/kd/pspbtknf_03g.bin

DC8/kd/pspbtlnf_03g.bin: bootcnf/bootcnf cfg/pspbtlnf_03g.txt
	./bootcnf/bootcnf -it cfg/pspbtlnf_03g.txt -ob DC8/kd/pspbtlnf_03g.bin

DC8/kd/pspbtrnf_03g.bin: bootcnf/bootcnf cfg/pspbtrnf_03g.txt
	./bootcnf/bootcnf -it cfg/pspbtrnf_03g.txt -ob DC8/kd/pspbtrnf_03g.bin
	
DC8/tmctrl500.prx:
	make -C ipl/rebootex clean
	make -C ipl/rebootex BFLAGS=-DIPL_01G
	cat ipl/rebootex/rebootex.bin | gzip > rebootex.bin.gz
	$(PSPDEV)/bin/bin2s rebootex.bin.gz modules/flashemu/rebootex.S rebootex
	rm rebootex.bin.gz

	make -C ipl/rebootex clean
	make -C ipl/rebootex BFLAGS=-DIPL_02G
	cat ipl/rebootex/rebootex.bin | gzip > rebootex_02g.bin.gz
	$(PSPDEV)/bin/bin2s rebootex_02g.bin.gz modules/flashemu/rebootex_02g.S rebootex_02g
	rm rebootex_02g.bin.gz

	make -C ipl/rebootex clean
	make -C ipl/rebootex BFLAGS=-DIPL_03G
	cat ipl/rebootex/rebootex.bin | gzip > rebootex_03g.bin.gz
	$(PSPDEV)/bin/bin2s rebootex_03g.bin.gz modules/flashemu/rebootex_03g.S rebootex_03g
	rm rebootex_03g.bin.gz

	make -C modules/flashemu
	python3 psptools/pack_kernel_module.py modules/flashemu/flashemu.prx DC8/tmctrl500.prx --tag 0x4c9416f0
	

DC8/kd/ipl_update.prx:
	make -C modules/ipl_update
	python3 psptools/pack_kernel_module.py modules/ipl_update/ipl_update.prx DC8/kd/ipl_update.prx --tag 0x4c9416f0
	
DC8/kd/resurrection.prx:
	make -C modules/vunbricker
	python3 psptools/pack_kernel_module.py modules/vunbricker/resurrection.prx DC8/kd/resurrection.prx --tag 0x38020af0


DC8/kd/dcman.prx:
	make -C modules/dcman
	python3 psptools/pack_kernel_module.py modules/dcman/dcman.prx DC8/kd/dcman.prx --tag 0x4c9416f0

DC8/kd/iop.prx:
	make -C modules/iop
	python3 psptools/pack_kernel_module.py modules/iop/iop.prx DC8/kd/iop.prx --tag 0x4c9416f0

DC8/kd/lflash_fdisk.prx:
	make -C modules/lflash_fdisk
	python3 psptools/pack_kernel_module.py modules/lflash_fdisk/lflash_fdisk.prx DC8/kd/lflash_fdisk.prx --tag 0x4c9416f0

DC8/kd/idsregeneration.prx:
	make -C modules/idsregeneration
	python3 psptools/pack_kernel_module.py modules/idsregeneration/idsregeneration.prx DC8/kd/idsregeneration.prx --tag 0x4c9416f0
	
DC8/kd/emc_sm_updater.prx:
	python3 psptools/pack_kernel_module.py modules/emc_sm_updater.prx DC8/kd/emc_sm_updater.prx --tag 0x4c9416f0

DC8/kd/lfatfs_updater.prx:
	python3 psptools/pack_kernel_module.py modules/lfatfs_updater.prx DC8/kd/lfatfs_updater.prx --tag 0x4c9416f0

DC8/kd/lflash_fatfmt_updater.prx:
	python3 psptools/pack_kernel_module.py modules/lflash_fatfmt_updater.prx DC8/kd/lflash_fatfmt_updater.prx --tag 0x4c9416f0

DC8/vsh/module/intraFont.prx:
	python3 psptools/pack_kernel_module.py modules/intraFont.elf DC8/vsh/module/intraFont.prx --tag 0x457b0af0

DC8/vsh/module/vlf.prx:
	python3 psptools/pack_kernel_module.py modules/vlf.elf DC8/vsh/module/vlf.prx --tag 0x457b0af0

DC8/kd/pspdecrypt.prx:
	python3 psptools/pack_kernel_module.py modules/pspdecrypt.elf DC8/kd/pspdecrypt.prx --tag 0x4c9416f0

DC8/kd/galaxy.prx:
	python3 psptools/pack_kernel_module.py modules/galaxy.elf DC8/kd/galaxy.prx --tag 0x4c9416f0

DC8/kd/idcanager.prx:
	python3 psptools/pack_kernel_module.py modules/idcanager.elf DC8/kd/idcanager.prx --tag 0x4c9416f0
	
DC8/kd/march33.prx:
	python3 psptools/pack_kernel_module.py modules/march33.elf DC8/kd/march33.prx --tag 0x4c9416f0
	
DC8/kd/popcorn.prx:
	python3 psptools/pack_kernel_module.py modules/popcorn.elf DC8/kd/popcorn.prx --tag 0x4c9416f0
	
DC8/kd/systemctrl.prx:
	python3 psptools/pack_kernel_module.py modules/systemctrl.elf DC8/kd/systemctrl.prx --tag 0x4c9416f0
	
DC8/kd/systemctrl_02g.prx:
	python3 psptools/pack_kernel_module.py modules/systemctrl_02g.elf DC8/kd/systemctrl_02g.prx --tag 0x4C9417F0

DC8/kd/systemctrl_03g.prx:
	python3 psptools/pack_kernel_module.py modules/systemctrl_03g.elf DC8/kd/systemctrl_03g.prx --tag 0x4C941FF0

DC8/kd/usbdevice.prx:
	python3 psptools/pack_kernel_module.py modules/usbdevice.elf DC8/kd/usbdevice.prx --tag 0x4c9416f0

DC8/kd/vshctrl.prx:
	python3 psptools/pack_kernel_module.py modules/vshctrl.elf DC8/kd/vshctrl.prx --tag 0x4c9416f0

DC8/vsh/module/recovery.prx:
	python3 psptools/pack_kernel_module.py modules/recovery.elf DC8/vsh/module/recovery.prx --tag 0x38020af0

DC8/vsh/module/satelite.prx:
	python3 psptools/pack_kernel_module.py modules/satelite.elf DC8/vsh/module/satelite.prx --tag 0x457b0af0


