PSPDEV=$(shell psp-config --pspdev-path)

all: TM/DC9 TM/msipl.bin TM/DC9/ipl_01g.bin TM/DC9/ipl_02g.bin TM/DC9/ipl_03g.bin TM/DC9/nandcipl_01g.bin TM/DC9/nandcipl_02g.bin TM/DC9/nandcipl_03g.bin TM/DC9/kd/pspbtcnf_dc.bin TM/DC9/kd/pspbtcnf_umd.bin TM/DC9/kd/pspbtcnf_oe.bin TM/DC9/kd/pspbtcnf_m33.bin TM/DC9/kd/pspbtcnf_recovery.bin TM/DC9/kd/pspbtcnf_02g_dc.bin TM/DC9/kd/pspbtcnf_02g_umd.bin TM/DC9/kd/pspbtcnf_02g_oe.bin TM/DC9/kd/pspbtcnf_02g_m33.bin TM/DC9/kd/pspbtcnf_02g_recovery.bin TM/DC9/kd/pspbtcnf_03g_dc.bin TM/DC9/kd/pspbtcnf_03g_umd.bin TM/DC9/kd/pspbtcnf_03g_oe.bin TM/DC9/kd/pspbtcnf_03g_m33.bin TM/DC9/kd/pspbtcnf_03g_recovery.bin TM/DC9/tmctrl.prx TM/DC9/kd/ipl_update.prx TM/DC9/kd/resurrection.prx TM/DC9/kd/dcman.prx TM/DC9/kd/iop.prx TM/DC9/kd/lflash_fdisk.prx TM/DC9/kd/idsregeneration.prx TM/DC9/kd/emc_sm_updater.prx TM/DC9/kd/lfatfs_updater.prx TM/DC9/kd/lflash_fatfmt_updater.prx TM/DC9/vsh/module/intraFont.prx TM/DC9/vsh/module/vlf.prx TM/DC9/kd/pspdecrypt.prx TM/DC9/kd/galaxy.prx TM/DC9/kd/idcanager.prx TM/DC9/kd/march33.prx TM/DC9/kd/popcorn.prx TM/DC9/kd/systemctrl.prx TM/DC9/kd/systemctrl_02g.prx TM/DC9/kd/systemctrl_03g.prx TM/DC9/kd/usbdevice.prx TM/DC9/kd/vshctrl.prx TM/DC9/vsh/module/recovery.prx TM/DC9/vsh/module/satelite.prx

clean:
	rm -rf TM
	make -C ipl/msipl clean
	make -C ipl/payloadex clean
	make -C ipl/ipl_stage2_payload clean
	rm -f ipl/ipl_stage2_payload/payloadex.s
	make -C ipl/ipl_stage1_payload clean
	rm -f ipl/ipl_stage1_payload/payload.S
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
	make -C modules/idcanager clean
	make -C modules/vshctrl clean
	make -C modules/usbdevice clean
	make -C modules/systemctrl clean
	rm -f modules/systemctrl/rebootex.S

TM:
	mkdir TM

TM/DC9: TM
	mkdir TM/DC9
	cp -R 502/flash0/* TM/DC9/
	cp -R 502/nandipl*.bin TM/DC9/
	mkdir -p TM/DC9/dic
	mkdir -p TM/DC9/gps
	mkdir -p TM/DC9/net/http
	mkdir -p TM/DC9/registry
	mkdir -p TM/DC9/vsh/theme
	touch TM/DC9/registry/init.dat

TM/msipl.bin: TM
	make -C ipl/msipl

	psptools/pack_ipl.py ipl/msipl/msipl.bin@0x40c0000 TM/msipl.bin 0x40c0000

TM/DC9/ipl_01g.bin: TM/DC9
	make -C ipl/payloadex clean
	make -C ipl/payloadex BFLAGS="-DIPL_01G -DMSIPL=1"
	$(PSPDEV)/bin/bin2s ipl/payloadex/payloadex.bin ipl/ipl_stage2_payload/payloadex.s payloadex

	make -C ipl/ipl_stage2_payload clean
	make -C ipl/ipl_stage2_payload CFLAGS="-DIPL_01G -DMSIPL=1"
	$(PSPDEV)/bin/bin2s ipl/ipl_stage2_payload/ipl_stage2_payload.bin ipl/ipl_stage1_payload/payload.S payload

	make -C ipl/ipl_stage1_payload clean
	make -C ipl/ipl_stage1_payload BFLAGS="-DIPL_01G -DMSIPL=1"

	dd if=ipl/ipl_stage1_payload/ipl_stage1_payload.bin of=TM/DC9/ipl_01g.bin

	psptools/unpack_ipl.py 502/nandipl_01g.bin ipl_01g.dec
	dd if=ipl_01g.dec of=TM/DC9/ipl_01g.bin bs=1 seek=12288
	rm ipl_01g.dec
	
TM/DC9/ipl_02g.bin: TM/DC9
	make -C ipl/payloadex clean
	make -C ipl/payloadex BFLAGS="-DIPL_02G -DMSIPL=1"
	$(PSPDEV)/bin/bin2s ipl/payloadex/payloadex.bin ipl/ipl_stage2_payload/payloadex.s payloadex

	make -C ipl/ipl_stage2_payload clean
	make -C ipl/ipl_stage2_payload CFLAGS="-DIPL_02G -DMSIPL=1"
	$(PSPDEV)/bin/bin2s ipl/ipl_stage2_payload/ipl_stage2_payload.bin ipl/ipl_stage1_payload/payload.S payload

	make -C ipl/ipl_stage1_payload clean
	make -C ipl/ipl_stage1_payload BFLAGS="-DIPL_02G -DMSIPL=1"

	dd if=ipl/ipl_stage1_payload/ipl_stage1_payload.bin of=TM/DC9/ipl_02g.bin

	psptools/unpack_ipl.py 502/nandipl_02g.bin ipl_02g.dec
	dd if=ipl_02g.dec of=TM/DC9/ipl_02g.bin bs=1 seek=12288
	rm ipl_02g.dec
	
TM/DC9/ipl_03g.bin: TM/DC9
	make -C ipl/payloadex clean
	make -C ipl/payloadex BFLAGS="-DIPL_03G -DMSIPL=1"
	$(PSPDEV)/bin/bin2s ipl/payloadex/payloadex.bin ipl/ipl_stage2_payload/payloadex.s payloadex

	make -C ipl/ipl_stage2_payload clean
	make -C ipl/ipl_stage2_payload CFLAGS="-DIPL_03G -DMSIPL=1"
	$(PSPDEV)/bin/bin2s ipl/ipl_stage2_payload/ipl_stage2_payload.bin ipl/ipl_stage1_payload/payload.S payload

	make -C ipl/ipl_stage1_payload clean
	make -C ipl/ipl_stage1_payload BFLAGS="-DIPL_03G -DMSIPL=1"

	dd if=ipl/ipl_stage1_payload/ipl_stage1_payload.bin of=TM/DC9/ipl_03g.bin
	psptools/unpack_ipl.py 502/nandipl_03g.bin ipl_03g.dec --xor 1
	dd if=ipl_03g.dec of=TM/DC9/ipl_03g.bin bs=1 seek=12288
	rm ipl_03g.dec

TM/DC9/nandcipl_01g.bin: TM/DC9
	make -C ipl/payloadex clean
	make -C ipl/payloadex BFLAGS="-DIPL_01G"
	$(PSPDEV)/bin/bin2s ipl/payloadex/payloadex.bin ipl/ipl_stage2_payload/payloadex.s payloadex

	make -C ipl/ipl_stage2_payload clean
	make -C ipl/ipl_stage2_payload CFLAGS="-DIPL_01G"
	$(PSPDEV)/bin/bin2s ipl/ipl_stage2_payload/ipl_stage2_payload.bin ipl/ipl_stage1_payload/payload.S payload

	make -C ipl/ipl_stage1_payload clean
	make -C ipl/ipl_stage1_payload BFLAGS="-DIPL_01G"

	psptools/unpack_ipl.py 502/nandipl_01g.bin ipl_01g.dec
	psptools/pack_ipl.py ipl/ipl_stage1_payload/ipl_stage1_payload.bin@0x40e0000 ipl_01g.dec@0x40f0000 TM/DC9/nandcipl_01g.bin 0x40e0000
	rm ipl_01g.dec


TM/DC9/nandcipl_02g.bin: TM/DC9
	make -C ipl/payloadex clean
	make -C ipl/payloadex BFLAGS="-DIPL_02G"
	$(PSPDEV)/bin/bin2s ipl/payloadex/payloadex.bin ipl/ipl_stage2_payload/payloadex.s payloadex

	make -C ipl/ipl_stage2_payload clean
	make -C ipl/ipl_stage2_payload CFLAGS="-DIPL_02G"
	$(PSPDEV)/bin/bin2s ipl/ipl_stage2_payload/ipl_stage2_payload.bin ipl/ipl_stage1_payload/payload.S payload

	make -C ipl/ipl_stage1_payload clean
	make -C ipl/ipl_stage1_payload BFLAGS="-DIPL_02G"

	psptools/unpack_ipl.py 502/nandipl_02g.bin ipl_02g.dec
	psptools/pack_ipl.py ipl/ipl_stage1_payload/ipl_stage1_payload.bin@0x40e0000 ipl_02g.dec@0x40f0000 TM/DC9/nandcipl_02g.bin 0x40e0000
	rm ipl_02g.dec

TM/DC9/nandcipl_03g.bin: TM/DC9
	make -C ipl/payloadex clean
	make -C ipl/payloadex BFLAGS="-DIPL_03G"
	$(PSPDEV)/bin/bin2s ipl/payloadex/payloadex.bin ipl/ipl_stage2_payload/payloadex.s payloadex

	make -C ipl/ipl_stage2_payload clean
	make -C ipl/ipl_stage2_payload CFLAGS="-DIPL_03G"
	$(PSPDEV)/bin/bin2s ipl/ipl_stage2_payload/ipl_stage2_payload.bin ipl/ipl_stage1_payload/payload.S payload

	make -C ipl/ipl_stage1_payload clean
	make -C ipl/ipl_stage1_payload BFLAGS="-DIPL_03G"

	psptools/unpack_ipl.py 502/nandipl_03g.bin ipl_03g.dec --xor 1
	psptools/pack_ipl.py ipl/ipl_stage1_payload/ipl_stage1_payload.bin@0x40e0000 ipl_03g.dec@0x40f0000 TM/DC9/nandcipl_03g.bin 0x40e0000
	rm ipl_03g.dec

bootcnf/bootcnf:
	make -C bootcnf

TM/DC9/kd/pspbtcnf_dc.bin: bootcnf/bootcnf cfg/pspbtcnf_dc.txt TM/DC9
	bootcnf/bootcnf -it cfg/pspbtcnf_dc.txt -ob pspbtcnf_dc.bin
	psptools/pack_bootcfg.py pspbtcnf_dc.bin TM/DC9/kd/pspbtcnf_dc.bin
	rm pspbtcnf_dc.bin

TM/DC9/kd/pspbtcnf_umd.bin: bootcnf/bootcnf cfg/pspbtcnf_umd.txt TM/DC9
	bootcnf/bootcnf -it cfg/pspbtcnf_umd.txt -ob pspbtcnf_umd.bin
	psptools/pack_bootcfg.py pspbtcnf_umd.bin TM/DC9/kd/pspbtcnf_umd.bin
	rm pspbtcnf_umd.bin

TM/DC9/kd/pspbtcnf_oe.bin: bootcnf/bootcnf cfg/pspbtcnf_oe.txt TM/DC9
	bootcnf/bootcnf -it cfg/pspbtcnf_oe.txt -ob pspbtcnf_oe.bin
	psptools/pack_bootcfg.py pspbtcnf_oe.bin TM/DC9/kd/pspbtcnf_oe.bin
	rm pspbtcnf_oe.bin

TM/DC9/kd/pspbtcnf_m33.bin: bootcnf/bootcnf cfg/pspbtcnf_m33.txt TM/DC9
	bootcnf/bootcnf -it cfg/pspbtcnf_m33.txt -ob pspbtcnf_m33.bin
	psptools/pack_bootcfg.py pspbtcnf_m33.bin TM/DC9/kd/pspbtcnf_m33.bin
	rm pspbtcnf_m33.bin

TM/DC9/kd/pspbtcnf_recovery.bin: bootcnf/bootcnf cfg/pspbtcnf_recovery.txt TM/DC9
	bootcnf/bootcnf -it cfg/pspbtcnf_recovery.txt -ob pspbtcnf_recovery.bin
	psptools/pack_bootcfg.py pspbtcnf_recovery.bin TM/DC9/kd/pspbtcnf_recovery.bin
	rm pspbtcnf_recovery.bin

TM/DC9/kd/pspbtcnf_02g_dc.bin: bootcnf/bootcnf cfg/pspbtcnf_02g_dc.txt TM/DC9
	bootcnf/bootcnf -it cfg/pspbtcnf_02g_dc.txt -ob pspbtcnf_02g_dc.bin
	psptools/pack_bootcfg.py pspbtcnf_02g_dc.bin TM/DC9/kd/pspbtcnf_02g_dc.bin
	rm pspbtcnf_02g_dc.bin

TM/DC9/kd/pspbtcnf_02g_umd.bin: bootcnf/bootcnf cfg/pspbtcnf_02g_umd.txt TM/DC9
	bootcnf/bootcnf -it cfg/pspbtcnf_02g_umd.txt -ob pspbtcnf_02g_umd.bin
	psptools/pack_bootcfg.py pspbtcnf_02g_umd.bin TM/DC9/kd/pspbtcnf_02g_umd.bin
	rm pspbtcnf_02g_umd.bin

TM/DC9/kd/pspbtcnf_02g_oe.bin: bootcnf/bootcnf cfg/pspbtcnf_02g_oe.txt TM/DC9
	bootcnf/bootcnf -it cfg/pspbtcnf_02g_oe.txt -ob pspbtcnf_02g_oe.bin
	psptools/pack_bootcfg.py pspbtcnf_02g_oe.bin TM/DC9/kd/pspbtcnf_02g_oe.bin
	rm pspbtcnf_02g_oe.bin

TM/DC9/kd/pspbtcnf_02g_m33.bin: bootcnf/bootcnf cfg/pspbtcnf_02g_m33.txt TM/DC9
	bootcnf/bootcnf -it cfg/pspbtcnf_02g_m33.txt -ob pspbtcnf_02g_m33.bin
	psptools/pack_bootcfg.py pspbtcnf_02g_m33.bin TM/DC9/kd/pspbtcnf_02g_m33.bin
	rm pspbtcnf_02g_m33.bin

TM/DC9/kd/pspbtcnf_02g_recovery.bin: bootcnf/bootcnf cfg/pspbtcnf_02g_recovery.txt TM/DC9
	bootcnf/bootcnf -it cfg/pspbtcnf_02g_recovery.txt -ob pspbtcnf_02g_recovery.bin
	psptools/pack_bootcfg.py pspbtcnf_02g_recovery.bin TM/DC9/kd/pspbtcnf_02g_recovery.bin
	rm pspbtcnf_02g_recovery.bin

TM/DC9/kd/pspbtcnf_03g_dc.bin: bootcnf/bootcnf cfg/pspbtcnf_03g_dc.txt TM/DC9
	bootcnf/bootcnf -it cfg/pspbtcnf_03g_dc.txt -ob pspbtcnf_03g_dc.bin
	psptools/pack_bootcfg.py pspbtcnf_03g_dc.bin TM/DC9/kd/pspbtcnf_03g_dc.bin
	rm pspbtcnf_03g_dc.bin

TM/DC9/kd/pspbtcnf_03g_umd.bin: bootcnf/bootcnf cfg/pspbtcnf_03g_umd.txt TM/DC9
	bootcnf/bootcnf -it cfg/pspbtcnf_03g_umd.txt -ob pspbtcnf_03g_umd.bin
	psptools/pack_bootcfg.py pspbtcnf_03g_umd.bin TM/DC9/kd/pspbtcnf_03g_umd.bin
	rm pspbtcnf_03g_umd.bin

TM/DC9/kd/pspbtcnf_03g_oe.bin: bootcnf/bootcnf cfg/pspbtcnf_03g_oe.txt TM/DC9
	bootcnf/bootcnf -it cfg/pspbtcnf_03g_oe.txt -ob pspbtcnf_03g_oe.bin
	psptools/pack_bootcfg.py pspbtcnf_03g_oe.bin TM/DC9/kd/pspbtcnf_03g_oe.bin
	rm pspbtcnf_03g_oe.bin

TM/DC9/kd/pspbtcnf_03g_m33.bin: bootcnf/bootcnf cfg/pspbtcnf_03g_m33.txt TM/DC9
	bootcnf/bootcnf -it cfg/pspbtcnf_03g_m33.txt -ob pspbtcnf_03g_m33.bin
	psptools/pack_bootcfg.py pspbtcnf_03g_m33.bin TM/DC9/kd/pspbtcnf_03g_m33.bin
	rm pspbtcnf_03g_m33.bin

TM/DC9/kd/pspbtcnf_03g_recovery.bin: bootcnf/bootcnf cfg/pspbtcnf_03g_recovery.txt TM/DC9
	bootcnf/bootcnf -it cfg/pspbtcnf_03g_recovery.txt -ob pspbtcnf_03g_recovery.bin
	psptools/pack_bootcfg.py pspbtcnf_03g_recovery.bin TM/DC9/kd/pspbtcnf_03g_recovery.bin
	rm pspbtcnf_03g_recovery.bin
	
TM/DC9/tmctrl.prx: TM/DC9
	make -C ipl/rebootex clean
	make -C ipl/rebootex BFLAGS="-DIPL_01G -DMSIPL=1"
	cat ipl/rebootex/rebootex.bin | gzip > rebootex.bin.gz
	$(PSPDEV)/bin/bin2s rebootex.bin.gz modules/flashemu/rebootex.S rebootex
	rm rebootex.bin.gz

	make -C ipl/rebootex clean
	make -C ipl/rebootex BFLAGS="-DIPL_02G -DMSIPL=1"
	cat ipl/rebootex/rebootex.bin | gzip > rebootex_02g.bin.gz
	$(PSPDEV)/bin/bin2s rebootex_02g.bin.gz modules/flashemu/rebootex_02g.S rebootex_02g
	rm rebootex_02g.bin.gz

	make -C ipl/rebootex clean
	make -C ipl/rebootex BFLAGS="-DIPL_03G -DMSIPL=1"
	cat ipl/rebootex/rebootex.bin | gzip > rebootex_03g.bin.gz
	$(PSPDEV)/bin/bin2s rebootex_03g.bin.gz modules/flashemu/rebootex_03g.S rebootex_03g
	rm rebootex_03g.bin.gz

	make -C modules/flashemu
	python3 psptools/pack_module.py modules/flashemu/flashemu.prx TM/DC9/tmctrl.prx --tag 0x4c9416f0
	
TM/DC9/kd/ipl_update.prx: TM/DC9
	make -C modules/ipl_update
	python3 psptools/pack_module.py modules/ipl_update/ipl_update.prx TM/DC9/kd/ipl_update.prx --tag 0x4c9416f0

TM/DC9/kd/resurrection.prx: TM/DC9
	make -C modules/vunbricker
	python3 psptools/pack_module.py modules/vunbricker/resurrection.prx TM/DC9/kd/resurrection.prx --tag 0x38020af0

TM/DC9/kd/dcman.prx: TM/DC9
	make -C modules/dcman
	python3 psptools/pack_module.py modules/dcman/dcman.prx TM/DC9/kd/dcman.prx --tag 0x4c9416f0

TM/DC9/kd/iop.prx: TM/DC9
	make -C modules/iop
	python3 psptools/pack_module.py modules/iop/iop.prx TM/DC9/kd/iop.prx --tag 0x4c9416f0

TM/DC9/kd/lflash_fdisk.prx: TM/DC9
	make -C modules/lflash_fdisk
	python3 psptools/pack_module.py modules/lflash_fdisk/lflash_fdisk.prx TM/DC9/kd/lflash_fdisk.prx --tag 0x4c9416f0

TM/DC9/kd/idsregeneration.prx: TM/DC9
	make -C modules/idsregeneration
	python3 psptools/pack_module.py modules/idsregeneration/idsregeneration.prx TM/DC9/kd/idsregeneration.prx --tag 0x4c9416f0
	
TM/DC9/kd/emc_sm_updater.prx: TM/DC9
	python3 psptools/pack_module.py modules/emc_sm_updater.prx TM/DC9/kd/emc_sm_updater.prx --tag 0x4c9416f0

TM/DC9/kd/lfatfs_updater.prx: TM/DC9
	python3 psptools/pack_module.py modules/lfatfs_updater.prx TM/DC9/kd/lfatfs_updater.prx --tag 0x4c9416f0

TM/DC9/kd/lflash_fatfmt_updater.prx: TM/DC9
	python3 psptools/pack_module.py modules/lflash_fatfmt_updater.prx TM/DC9/kd/lflash_fatfmt_updater.prx --tag 0x4c9416f0

TM/DC9/vsh/module/intraFont.prx: TM/DC9
	python3 psptools/pack_module.py modules/intraFont.elf TM/DC9/vsh/module/intraFont.prx --tag 0x457b0af0

TM/DC9/vsh/module/vlf.prx: TM/DC9
	python3 psptools/pack_module.py modules/vlf.elf TM/DC9/vsh/module/vlf.prx --tag 0x457b0af0

TM/DC9/kd/pspdecrypt.prx: TM/DC9
	python3 psptools/pack_module.py modules/pspdecrypt.elf TM/DC9/kd/pspdecrypt.prx --tag 0x4c9416f0

TM/DC9/kd/galaxy.prx: TM/DC9
	python3 psptools/pack_module.py modules/galaxy.elf TM/DC9/kd/galaxy.prx --tag 0x4c9416f0

TM/DC9/kd/idcanager.prx: TM/DC9
	make -C modules/idcanager
	python3 psptools/pack_module.py modules/idcanager/idcanager.prx TM/DC9/kd/idcanager.prx --tag 0x4c9416f0
	
TM/DC9/kd/march33.prx: TM/DC9
	python3 psptools/pack_module.py modules/march33.elf TM/DC9/kd/march33.prx --tag 0x4c9416f0
	
TM/DC9/kd/popcorn.prx: TM/DC9
	python3 psptools/pack_module.py modules/popcorn.elf TM/DC9/kd/popcorn.prx --tag 0x4c9416f0
	
TM/DC9/kd/systemctrl.prx: TM/DC9
	make -C ipl/rebootex clean
	make -C ipl/rebootex BFLAGS="-DIPL_01G"
	cat ipl/rebootex/rebootex.bin | gzip > rebootex.bin.gz
	$(PSPDEV)/bin/bin2s rebootex.bin.gz modules/systemctrl/rebootex.S rebootex
	rm rebootex.bin.gz
	
	make -C modules/systemctrl
	python3 psptools/pack_module.py modules/systemctrl/systemctrl.prx TM/DC9/kd/systemctrl.prx --tag 0x4c9416f0
	
TM/DC9/kd/systemctrl_02g.prx: TM/DC9
	make -C ipl/rebootex clean
	make -C ipl/rebootex BFLAGS="-DIPL_02G"
	cat ipl/rebootex/rebootex.bin | gzip > rebootex.bin.gz
	$(PSPDEV)/bin/bin2s rebootex.bin.gz modules/systemctrl/rebootex.S rebootex
	rm rebootex.bin.gz
	
	make -C modules/systemctrl
	python3 psptools/pack_module.py modules/systemctrl/systemctrl.prx TM/DC9/kd/systemctrl_02g.prx --tag 0x4c9417f0

TM/DC9/kd/systemctrl_03g.prx: TM/DC9
	make -C ipl/rebootex clean
	make -C ipl/rebootex BFLAGS="-DIPL_03G"
	cat ipl/rebootex/rebootex.bin | gzip > rebootex.bin.gz
	$(PSPDEV)/bin/bin2s rebootex.bin.gz modules/systemctrl/rebootex.S rebootex
	rm rebootex.bin.gz
	
	make -C modules/systemctrl
	python3 psptools/pack_module.py modules/systemctrl/systemctrl.prx TM/DC9/kd/systemctrl_03g.prx --tag 0x4c941ff0

TM/DC9/kd/usbdevice.prx: TM/DC9
	make -C modules/usbdevice
	python3 psptools/pack_module.py modules/usbdevice/usbdevice.prx TM/DC9/kd/usbdevice.prx --tag 0x4c9416f0

TM/DC9/kd/vshctrl.prx: TM/DC9
	make -C modules/vshctrl
	python3 psptools/pack_module.py modules/vshctrl/vshctrl.prx TM/DC9/kd/vshctrl.prx --tag 0x4c9416f0

TM/DC9/vsh/module/recovery.prx: TM/DC9
	python3 psptools/pack_module.py modules/recovery.elf TM/DC9/vsh/module/recovery.prx --tag 0x38020af0

TM/DC9/vsh/module/satelite.prx: TM/DC9
	python3 psptools/pack_module.py modules/satelite.elf TM/DC9/vsh/module/satelite.prx --tag 0x457b0af0


