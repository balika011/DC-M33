PSPDEV=$(shell psp-config --pspdev-path)

all: TM/DC10 TM/msipl.bin \
TM/DC10/ipl_01g.bin TM/DC10/ipl_02g.bin TM/DC10/ipl_03g.bin TM/DC10/ipl_04g.bin TM/DC10/ipl_05g.bin TM/DC10/ipl_07g.bin TM/DC10/ipl_09g.bin TM/DC10/ipl_11g.bin \
TM/DC10/nandcipl_01g.bin TM/DC10/nandcipl_02g.bin TM/DC10/nandcipl_03g.bin TM/DC10/nandcipl_04g.bin TM/DC10/nandcipl_05g.bin TM/DC10/nandcipl_07g.bin TM/DC10/nandcipl_09g.bin TM/DC10/nandcipl_11g.bin \
TM/DC10/retail/kd/pspbtcnf_dc.bin TM/DC10/retail/kd/pspbtcnf_umd.bin TM/DC10/retail/kd/pspbtcnf_np.bin TM/DC10/retail/kd/pspbtcnf_m33.bin TM/DC10/retail/kd/pspbtcnf_recovery.bin \
TM/DC10/retail/kd/pspbtcnf_02g_dc.bin TM/DC10/retail/kd/pspbtcnf_02g_umd.bin TM/DC10/retail/kd/pspbtcnf_02g_np.bin TM/DC10/retail/kd/pspbtcnf_02g_m33.bin TM/DC10/retail/kd/pspbtcnf_02g_recovery.bin \
TM/DC10/retail/kd/pspbtcnf_03g_dc.bin TM/DC10/retail/kd/pspbtcnf_03g_umd.bin TM/DC10/retail/kd/pspbtcnf_03g_np.bin TM/DC10/retail/kd/pspbtcnf_03g_m33.bin TM/DC10/retail/kd/pspbtcnf_03g_recovery.bin \
TM/DC10/retail/kd/pspbtcnf_04g_dc.bin TM/DC10/retail/kd/pspbtcnf_04g_umd.bin TM/DC10/retail/kd/pspbtcnf_04g_np.bin TM/DC10/retail/kd/pspbtcnf_04g_m33.bin TM/DC10/retail/kd/pspbtcnf_04g_recovery.bin \
TM/DC10/retail/kd/pspbtcnf_09g_dc.bin TM/DC10/retail/kd/pspbtcnf_09g_umd.bin TM/DC10/retail/kd/pspbtcnf_09g_np.bin TM/DC10/retail/kd/pspbtcnf_09g_m33.bin TM/DC10/retail/kd/pspbtcnf_09g_recovery.bin \
TM/DC10/retail/kd/pspbtcnf_11g_dc.bin TM/DC10/retail/kd/pspbtcnf_11g_umd.bin TM/DC10/retail/kd/pspbtcnf_11g_np.bin TM/DC10/retail/kd/pspbtcnf_11g_m33.bin TM/DC10/retail/kd/pspbtcnf_11g_recovery.bin \
TM/DC10/testingtool/kd/pspbtcnf_dc.bin TM/DC10/testingtool/kd/pspbtcnf_umd.bin TM/DC10/testingtool/kd/pspbtcnf_np.bin TM/DC10/testingtool/kd/pspbtcnf_m33.bin TM/DC10/testingtool/kd/pspbtcnf_recovery.bin \
TM/DC10/testingtool/kd/pspbtcnf_02g_dc.bin TM/DC10/testingtool/kd/pspbtcnf_02g_umd.bin TM/DC10/testingtool/kd/pspbtcnf_02g_np.bin TM/DC10/testingtool/kd/pspbtcnf_02g_m33.bin TM/DC10/testingtool/kd/pspbtcnf_02g_recovery.bin \
TM/DC10/testingtool/kd/pspbtcnf_03g_dc.bin TM/DC10/testingtool/kd/pspbtcnf_03g_umd.bin TM/DC10/testingtool/kd/pspbtcnf_03g_np.bin TM/DC10/testingtool/kd/pspbtcnf_03g_m33.bin TM/DC10/testingtool/kd/pspbtcnf_03g_recovery.bin \
TM/DC10/tmctrl.prx TM/DC10/kd/ipl_update.prx TM/DC10/kd/resurrection.prx TM/DC10/kd/dcman.prx TM/DC10/kd/iop.prx TM/DC10/kd/lflash_fdisk.prx TM/DC10/kd/idsregeneration.prx TM/DC10/kd/emc_sm_updater.prx \
TM/DC10/kd/lfatfs_updater.prx TM/DC10/kd/lflash_fatfmt_updater.prx TM/DC10/vsh/module/intraFont.prx TM/DC10/vsh/module/vlf.prx TM/DC10/kd/pspdecrypt.prx TM/DC10/kd/galaxy.prx TM/DC10/kd/idcanager.prx TM/DC10/kd/march33.prx \
TM/DC10/kd/popcorn.prx TM/DC10/kd/nidresolver.prx TM/DC10/kd/systemctrl.prx TM/DC10/kd/usbdevice.prx TM/DC10/kd/vshctrl.prx TM/DC10/vsh/module/recovery.prx TM/DC10/vsh/module/satelite.prx

clean:
	rm -rf TM
	make -C ipl/common clean
	make -C ipl/msipl clean
	make -C ipl/payloadex clean
	make -C ipl/ipl_stage2_payload clean
	rm -f ipl/ipl_stage2_payload/payloadex.S
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
	make -C modules/nidresolver clean
	make -C modules/systemctrl clean
	rm -f modules/systemctrl/rebootex.S
	rm -f modules/systemctrl/rebootex_02g.S
	rm -f modules/systemctrl/rebootex_03g.S
	make -C modules/satelite clean
	make -C modules/galaxy clean
	make -C modules/popcorn clean

TM:
	mkdir TM

TM/DC10: TM
	mkdir TM/DC10
	cp -R 661/flash0/* TM/DC10/
	cp -R 661/nandipl*.bin TM/DC10/
	mkdir -p TM/DC10/dic
	mkdir -p TM/DC10/gps
	mkdir -p TM/DC10/net/http
	mkdir -p TM/DC10/registry
	mkdir -p TM/DC10/vsh/module
	mkdir -p TM/DC10/vsh/theme
	mkdir -p TM/DC10/retail/kd
	mkdir -p TM/DC10/retail/vsh
	mkdir -p TM/DC10/testingtool/kd
	mkdir -p TM/DC10/testingtool/vsh
	touch TM/DC10/registry/init.dat

ipl/common/libiplsdk.a:
	make -C ipl/common

TM/msipl.bin: TM ipl/common/libiplsdk.a
	make -C ipl/msipl

	psptools/pack_ipl.py ipl/msipl/msipl.bin@0x40c0000 TM/msipl.bin 0x40c0000

TM/DC10/ipl_01g.bin: TM/DC10 ipl/common/libiplsdk.a
	make -C ipl/payloadex clean
	make -C ipl/payloadex BFLAGS="-DIPL_01G -DMSIPL=1"
	$(PSPDEV)/bin/bin2s ipl/payloadex/payloadex.bin ipl/ipl_stage2_payload/payloadex.S payloadex

	make -C ipl/ipl_stage2_payload clean
	make -C ipl/ipl_stage2_payload BFLAGS="-DIPL_01G -DMSIPL=1"
	$(PSPDEV)/bin/bin2s ipl/ipl_stage2_payload/ipl_stage2_payload.bin ipl/ipl_stage1_payload/payload.S payload

	make -C ipl/ipl_stage1_payload clean
	make -C ipl/ipl_stage1_payload BFLAGS="-DIPL_01G -DMSIPL=1"

	dd if=ipl/ipl_stage1_payload/ipl_stage1_payload.bin of=TM/DC10/ipl_01g.bin

	psptools/unpack_ipl.py 661/nandipl_01g.bin ipl_01g.dec
	dd if=ipl_01g.dec of=TM/DC10/ipl_01g.bin bs=1 seek=49152
	rm ipl_01g.dec
	
TM/DC10/ipl_02g.bin: TM/DC10 ipl/common/libiplsdk.a
	make -C ipl/payloadex clean
	make -C ipl/payloadex BFLAGS="-DIPL_02G -DMSIPL=1"
	$(PSPDEV)/bin/bin2s ipl/payloadex/payloadex.bin ipl/ipl_stage2_payload/payloadex.S payloadex

	make -C ipl/ipl_stage2_payload clean
	make -C ipl/ipl_stage2_payload BFLAGS="-DIPL_02G -DMSIPL=1"
	$(PSPDEV)/bin/bin2s ipl/ipl_stage2_payload/ipl_stage2_payload.bin ipl/ipl_stage1_payload/payload.S payload

	make -C ipl/ipl_stage1_payload clean
	make -C ipl/ipl_stage1_payload BFLAGS="-DIPL_02G -DMSIPL=1"

	dd if=ipl/ipl_stage1_payload/ipl_stage1_payload.bin of=TM/DC10/ipl_02g.bin

	psptools/unpack_ipl.py 661/nandipl_02g.bin ipl_02g.dec
	dd if=ipl_02g.dec of=TM/DC10/ipl_02g.bin bs=1 seek=49152
	rm ipl_02g.dec
	
TM/DC10/ipl_03g.bin: TM/DC10 ipl/common/libiplsdk.a
	make -C ipl/payloadex clean
	make -C ipl/payloadex BFLAGS="-DIPL_03G -DMSIPL=1"
	$(PSPDEV)/bin/bin2s ipl/payloadex/payloadex.bin ipl/ipl_stage2_payload/payloadex.S payloadex

	make -C ipl/ipl_stage2_payload clean
	make -C ipl/ipl_stage2_payload BFLAGS="-DIPL_03G -DMSIPL=1"
	$(PSPDEV)/bin/bin2s ipl/ipl_stage2_payload/ipl_stage2_payload.bin ipl/ipl_stage1_payload/payload.S payload

	make -C ipl/ipl_stage1_payload clean
	make -C ipl/ipl_stage1_payload BFLAGS="-DIPL_03G -DMSIPL=1"

	dd if=ipl/ipl_stage1_payload/ipl_stage1_payload.bin of=TM/DC10/ipl_03g.bin
	psptools/unpack_ipl.py 661/nandipl_03g.bin ipl_03g.dec --xor 1
	dd if=ipl_03g.dec of=TM/DC10/ipl_03g.bin bs=1 seek=49152
	rm ipl_03g.dec

TM/DC10/nandcipl_01g.bin: TM/DC10 ipl/common/libiplsdk.a
	make -C ipl/payloadex clean
	make -C ipl/payloadex BFLAGS="-DIPL_01G"
	$(PSPDEV)/bin/bin2s ipl/payloadex/payloadex.bin ipl/ipl_stage2_payload/payloadex.S payloadex

	make -C ipl/ipl_stage2_payload clean
	make -C ipl/ipl_stage2_payload BFLAGS="-DIPL_01G"
	$(PSPDEV)/bin/bin2s ipl/ipl_stage2_payload/ipl_stage2_payload.bin ipl/ipl_stage1_payload/payload.S payload

	make -C ipl/ipl_stage1_payload clean
	make -C ipl/ipl_stage1_payload BFLAGS="-DIPL_01G"

	psptools/unpack_ipl.py 661/nandipl_01g.bin ipl_01g.dec
	psptools/pack_ipl.py ipl/ipl_stage1_payload/ipl_stage1_payload.bin@0x40e0000 ipl_01g.dec@0x40f0000 TM/DC10/nandcipl_01g.bin 0x40e0000
	rm ipl_01g.dec

TM/DC10/nandcipl_02g.bin: TM/DC10 ipl/common/libiplsdk.a
	make -C ipl/payloadex clean
	make -C ipl/payloadex BFLAGS="-DIPL_02G"
	$(PSPDEV)/bin/bin2s ipl/payloadex/payloadex.bin ipl/ipl_stage2_payload/payloadex.S payloadex

	make -C ipl/ipl_stage2_payload clean
	make -C ipl/ipl_stage2_payload BFLAGS="-DIPL_02G"
	$(PSPDEV)/bin/bin2s ipl/ipl_stage2_payload/ipl_stage2_payload.bin ipl/ipl_stage1_payload/payload.S payload

	make -C ipl/ipl_stage1_payload clean
	make -C ipl/ipl_stage1_payload BFLAGS="-DIPL_02G"

	psptools/unpack_ipl.py 661/nandipl_02g.bin ipl_02g.dec
	psptools/pack_ipl.py ipl/ipl_stage1_payload/ipl_stage1_payload.bin@0x40e0000 ipl_02g.dec@0x40f0000 TM/DC10/nandcipl_02g.bin 0x40e0000
	rm ipl_02g.dec

TM/DC10/nandcipl_03g.bin: TM/DC10 ipl/common/libiplsdk.a
	make -C ipl/payloadex clean
	make -C ipl/payloadex BFLAGS="-DIPL_03G"
	$(PSPDEV)/bin/bin2s ipl/payloadex/payloadex.bin ipl/ipl_stage2_payload/payloadex.S payloadex

	make -C ipl/ipl_stage2_payload clean
	make -C ipl/ipl_stage2_payload BFLAGS="-DIPL_03G"
	$(PSPDEV)/bin/bin2s ipl/ipl_stage2_payload/ipl_stage2_payload.bin ipl/ipl_stage1_payload/payload.S payload

	make -C ipl/ipl_stage1_payload clean
	make -C ipl/ipl_stage1_payload BFLAGS="-DIPL_03G"

	psptools/unpack_ipl.py 661/nandipl_03g.bin ipl_03g.dec --xor 1
	psptools/pack_ipl.py ipl/ipl_stage1_payload/ipl_stage1_payload.bin@0x40e0000 ipl_03g.dec@0x40f0000 TM/DC10/nandcipl_03g.bin 0x40e0000
	rm ipl_03g.dec

bootcnf/bootcnf:
	make -C bootcnf

TM/DC10/retail/kd/pspbtcnf_dc.bin: bootcnf/bootcnf cfg_retail/pspbtcnf_dc.txt TM/DC10
	bootcnf/bootcnf -it cfg_retail/pspbtcnf_dc.txt -ob pspbtcnf_dc.bin
	psptools/pack_bootcfg.py pspbtcnf_dc.bin TM/DC10/retail/kd/pspbtcnf_dc.bin
	rm pspbtcnf_dc.bin

TM/DC10/retail/kd/pspbtcnf_umd.bin: bootcnf/bootcnf cfg_retail/pspbtcnf_umd.txt TM/DC10
	bootcnf/bootcnf -it cfg_retail/pspbtcnf_umd.txt -ob pspbtcnf_umd.bin
	psptools/pack_bootcfg.py pspbtcnf_umd.bin TM/DC10/retail/kd/pspbtcnf_umd.bin
	rm pspbtcnf_umd.bin

TM/DC10/retail/kd/pspbtcnf_np.bin: bootcnf/bootcnf cfg_retail/pspbtcnf_np.txt TM/DC10
	bootcnf/bootcnf -it cfg_retail/pspbtcnf_np.txt -ob pspbtcnf_np.bin
	psptools/pack_bootcfg.py pspbtcnf_np.bin TM/DC10/retail/kd/pspbtcnf_np.bin
	rm pspbtcnf_np.bin

TM/DC10/retail/kd/pspbtcnf_m33.bin: bootcnf/bootcnf cfg_retail/pspbtcnf_m33.txt TM/DC10
	bootcnf/bootcnf -it cfg_retail/pspbtcnf_m33.txt -ob pspbtcnf_m33.bin
	psptools/pack_bootcfg.py pspbtcnf_m33.bin TM/DC10/retail/kd/pspbtcnf_m33.bin
	rm pspbtcnf_m33.bin

TM/DC10/retail/kd/pspbtcnf_recovery.bin: bootcnf/bootcnf cfg_retail/pspbtcnf_recovery.txt TM/DC10
	bootcnf/bootcnf -it cfg_retail/pspbtcnf_recovery.txt -ob pspbtcnf_recovery.bin
	psptools/pack_bootcfg.py pspbtcnf_recovery.bin TM/DC10/retail/kd/pspbtcnf_recovery.bin
	rm pspbtcnf_recovery.bin

TM/DC10/retail/kd/pspbtcnf_02g_dc.bin: bootcnf/bootcnf cfg_retail/pspbtcnf_02g_dc.txt TM/DC10
	bootcnf/bootcnf -it cfg_retail/pspbtcnf_02g_dc.txt -ob pspbtcnf_02g_dc.bin
	psptools/pack_bootcfg.py pspbtcnf_02g_dc.bin TM/DC10/retail/kd/pspbtcnf_02g_dc.bin
	rm pspbtcnf_02g_dc.bin

TM/DC10/retail/kd/pspbtcnf_02g_umd.bin: bootcnf/bootcnf cfg_retail/pspbtcnf_02g_umd.txt TM/DC10
	bootcnf/bootcnf -it cfg_retail/pspbtcnf_02g_umd.txt -ob pspbtcnf_02g_umd.bin
	psptools/pack_bootcfg.py pspbtcnf_02g_umd.bin TM/DC10/retail/kd/pspbtcnf_02g_umd.bin
	rm pspbtcnf_02g_umd.bin

TM/DC10/retail/kd/pspbtcnf_02g_np.bin: bootcnf/bootcnf cfg_retail/pspbtcnf_02g_np.txt TM/DC10
	bootcnf/bootcnf -it cfg_retail/pspbtcnf_02g_np.txt -ob pspbtcnf_02g_np.bin
	psptools/pack_bootcfg.py pspbtcnf_02g_np.bin TM/DC10/retail/kd/pspbtcnf_02g_np.bin
	rm pspbtcnf_02g_np.bin

TM/DC10/retail/kd/pspbtcnf_02g_m33.bin: bootcnf/bootcnf cfg_retail/pspbtcnf_02g_m33.txt TM/DC10
	bootcnf/bootcnf -it cfg_retail/pspbtcnf_02g_m33.txt -ob pspbtcnf_02g_m33.bin
	psptools/pack_bootcfg.py pspbtcnf_02g_m33.bin TM/DC10/retail/kd/pspbtcnf_02g_m33.bin
	rm pspbtcnf_02g_m33.bin

TM/DC10/retail/kd/pspbtcnf_02g_recovery.bin: bootcnf/bootcnf cfg_retail/pspbtcnf_02g_recovery.txt TM/DC10
	bootcnf/bootcnf -it cfg_retail/pspbtcnf_02g_recovery.txt -ob pspbtcnf_02g_recovery.bin
	psptools/pack_bootcfg.py pspbtcnf_02g_recovery.bin TM/DC10/retail/kd/pspbtcnf_02g_recovery.bin
	rm pspbtcnf_02g_recovery.bin

TM/DC10/retail/kd/pspbtcnf_03g_dc.bin: bootcnf/bootcnf cfg_retail/pspbtcnf_03g_dc.txt TM/DC10
	bootcnf/bootcnf -it cfg_retail/pspbtcnf_03g_dc.txt -ob pspbtcnf_03g_dc.bin
	psptools/pack_bootcfg.py pspbtcnf_03g_dc.bin TM/DC10/retail/kd/pspbtcnf_03g_dc.bin
	rm pspbtcnf_03g_dc.bin

TM/DC10/retail/kd/pspbtcnf_03g_umd.bin: bootcnf/bootcnf cfg_retail/pspbtcnf_03g_umd.txt TM/DC10
	bootcnf/bootcnf -it cfg_retail/pspbtcnf_03g_umd.txt -ob pspbtcnf_03g_umd.bin
	psptools/pack_bootcfg.py pspbtcnf_03g_umd.bin TM/DC10/retail/kd/pspbtcnf_03g_umd.bin
	rm pspbtcnf_03g_umd.bin

TM/DC10/retail/kd/pspbtcnf_03g_np.bin: bootcnf/bootcnf cfg_retail/pspbtcnf_03g_np.txt TM/DC10
	bootcnf/bootcnf -it cfg_retail/pspbtcnf_03g_np.txt -ob pspbtcnf_03g_np.bin
	psptools/pack_bootcfg.py pspbtcnf_03g_np.bin TM/DC10/retail/kd/pspbtcnf_03g_np.bin
	rm pspbtcnf_03g_np.bin

TM/DC10/retail/kd/pspbtcnf_03g_m33.bin: bootcnf/bootcnf cfg_retail/pspbtcnf_03g_m33.txt TM/DC10
	bootcnf/bootcnf -it cfg_retail/pspbtcnf_03g_m33.txt -ob pspbtcnf_03g_m33.bin
	psptools/pack_bootcfg.py pspbtcnf_03g_m33.bin TM/DC10/retail/kd/pspbtcnf_03g_m33.bin
	rm pspbtcnf_03g_m33.bin

TM/DC10/retail/kd/pspbtcnf_03g_recovery.bin: bootcnf/bootcnf cfg_retail/pspbtcnf_03g_recovery.txt TM/DC10
	bootcnf/bootcnf -it cfg_retail/pspbtcnf_03g_recovery.txt -ob pspbtcnf_03g_recovery.bin
	psptools/pack_bootcfg.py pspbtcnf_03g_recovery.bin TM/DC10/retail/kd/pspbtcnf_03g_recovery.bin
	rm pspbtcnf_03g_recovery.bin

TM/DC10/retail/kd/pspbtcnf_04g_dc.bin: bootcnf/bootcnf cfg_retail/pspbtcnf_04g_dc.txt TM/DC10
	bootcnf/bootcnf -it cfg_retail/pspbtcnf_04g_dc.txt -ob pspbtcnf_04g_dc.bin
	psptools/pack_bootcfg.py pspbtcnf_04g_dc.bin TM/DC10/retail/kd/pspbtcnf_04g_dc.bin
	rm pspbtcnf_04g_dc.bin

TM/DC10/retail/kd/pspbtcnf_04g_umd.bin: bootcnf/bootcnf cfg_retail/pspbtcnf_04g_umd.txt TM/DC10
	bootcnf/bootcnf -it cfg_retail/pspbtcnf_04g_umd.txt -ob pspbtcnf_04g_umd.bin
	psptools/pack_bootcfg.py pspbtcnf_04g_umd.bin TM/DC10/retail/kd/pspbtcnf_04g_umd.bin
	rm pspbtcnf_04g_umd.bin

TM/DC10/retail/kd/pspbtcnf_04g_np.bin: bootcnf/bootcnf cfg_retail/pspbtcnf_04g_np.txt TM/DC10
	bootcnf/bootcnf -it cfg_retail/pspbtcnf_04g_np.txt -ob pspbtcnf_04g_np.bin
	psptools/pack_bootcfg.py pspbtcnf_04g_np.bin TM/DC10/retail/kd/pspbtcnf_04g_np.bin
	rm pspbtcnf_04g_np.bin

TM/DC10/retail/kd/pspbtcnf_04g_m33.bin: bootcnf/bootcnf cfg_retail/pspbtcnf_04g_m33.txt TM/DC10
	bootcnf/bootcnf -it cfg_retail/pspbtcnf_04g_m33.txt -ob pspbtcnf_04g_m33.bin
	psptools/pack_bootcfg.py pspbtcnf_04g_m33.bin TM/DC10/retail/kd/pspbtcnf_04g_m33.bin
	rm pspbtcnf_04g_m33.bin

TM/DC10/retail/kd/pspbtcnf_04g_recovery.bin: bootcnf/bootcnf cfg_retail/pspbtcnf_04g_recovery.txt TM/DC10
	bootcnf/bootcnf -it cfg_retail/pspbtcnf_04g_recovery.txt -ob pspbtcnf_04g_recovery.bin
	psptools/pack_bootcfg.py pspbtcnf_04g_recovery.bin TM/DC10/retail/kd/pspbtcnf_04g_recovery.bin
	rm pspbtcnf_04g_recovery.bin

TM/DC10/retail/kd/pspbtcnf_09g_dc.bin: bootcnf/bootcnf cfg_retail/pspbtcnf_09g_dc.txt TM/DC10
	bootcnf/bootcnf -it cfg_retail/pspbtcnf_09g_dc.txt -ob pspbtcnf_09g_dc.bin
	psptools/pack_bootcfg.py pspbtcnf_09g_dc.bin TM/DC10/retail/kd/pspbtcnf_09g_dc.bin
	rm pspbtcnf_09g_dc.bin

TM/DC10/retail/kd/pspbtcnf_09g_umd.bin: bootcnf/bootcnf cfg_retail/pspbtcnf_09g_umd.txt TM/DC10
	bootcnf/bootcnf -it cfg_retail/pspbtcnf_09g_umd.txt -ob pspbtcnf_09g_umd.bin
	psptools/pack_bootcfg.py pspbtcnf_09g_umd.bin TM/DC10/retail/kd/pspbtcnf_09g_umd.bin
	rm pspbtcnf_09g_umd.bin

TM/DC10/retail/kd/pspbtcnf_09g_np.bin: bootcnf/bootcnf cfg_retail/pspbtcnf_09g_np.txt TM/DC10
	bootcnf/bootcnf -it cfg_retail/pspbtcnf_09g_np.txt -ob pspbtcnf_09g_np.bin
	psptools/pack_bootcfg.py pspbtcnf_09g_np.bin TM/DC10/retail/kd/pspbtcnf_09g_np.bin
	rm pspbtcnf_09g_np.bin

TM/DC10/retail/kd/pspbtcnf_09g_m33.bin: bootcnf/bootcnf cfg_retail/pspbtcnf_09g_m33.txt TM/DC10
	bootcnf/bootcnf -it cfg_retail/pspbtcnf_09g_m33.txt -ob pspbtcnf_09g_m33.bin
	psptools/pack_bootcfg.py pspbtcnf_09g_m33.bin TM/DC10/retail/kd/pspbtcnf_09g_m33.bin
	rm pspbtcnf_09g_m33.bin

TM/DC10/retail/kd/pspbtcnf_09g_recovery.bin: bootcnf/bootcnf cfg_retail/pspbtcnf_09g_recovery.txt TM/DC10
	bootcnf/bootcnf -it cfg_retail/pspbtcnf_09g_recovery.txt -ob pspbtcnf_09g_recovery.bin
	psptools/pack_bootcfg.py pspbtcnf_09g_recovery.bin TM/DC10/retail/kd/pspbtcnf_09g_recovery.bin
	rm pspbtcnf_09g_recovery.bin

TM/DC10/retail/kd/pspbtcnf_11g_dc.bin: bootcnf/bootcnf cfg_retail/pspbtcnf_11g_dc.txt TM/DC10
	bootcnf/bootcnf -it cfg_retail/pspbtcnf_11g_dc.txt -ob pspbtcnf_11g_dc.bin
	psptools/pack_bootcfg.py pspbtcnf_11g_dc.bin TM/DC10/retail/kd/pspbtcnf_11g_dc.bin
	rm pspbtcnf_11g_dc.bin

TM/DC10/retail/kd/pspbtcnf_11g_umd.bin: bootcnf/bootcnf cfg_retail/pspbtcnf_11g_umd.txt TM/DC10
	bootcnf/bootcnf -it cfg_retail/pspbtcnf_11g_umd.txt -ob pspbtcnf_11g_umd.bin
	psptools/pack_bootcfg.py pspbtcnf_11g_umd.bin TM/DC10/retail/kd/pspbtcnf_11g_umd.bin
	rm pspbtcnf_11g_umd.bin

TM/DC10/retail/kd/pspbtcnf_11g_np.bin: bootcnf/bootcnf cfg_retail/pspbtcnf_11g_np.txt TM/DC10
	bootcnf/bootcnf -it cfg_retail/pspbtcnf_11g_np.txt -ob pspbtcnf_11g_np.bin
	psptools/pack_bootcfg.py pspbtcnf_11g_np.bin TM/DC10/retail/kd/pspbtcnf_11g_np.bin
	rm pspbtcnf_11g_np.bin

TM/DC10/retail/kd/pspbtcnf_11g_m33.bin: bootcnf/bootcnf cfg_retail/pspbtcnf_11g_m33.txt TM/DC10
	bootcnf/bootcnf -it cfg_retail/pspbtcnf_11g_m33.txt -ob pspbtcnf_11g_m33.bin
	psptools/pack_bootcfg.py pspbtcnf_11g_m33.bin TM/DC10/retail/kd/pspbtcnf_11g_m33.bin
	rm pspbtcnf_11g_m33.bin

TM/DC10/retail/kd/pspbtcnf_11g_recovery.bin: bootcnf/bootcnf cfg_retail/pspbtcnf_11g_recovery.txt TM/DC10
	bootcnf/bootcnf -it cfg_retail/pspbtcnf_11g_recovery.txt -ob pspbtcnf_11g_recovery.bin
	psptools/pack_bootcfg.py pspbtcnf_11g_recovery.bin TM/DC10/retail/kd/pspbtcnf_11g_recovery.bin
	rm pspbtcnf_11g_recovery.bin

TM/DC10/testingtool/kd/pspbtcnf_dc.bin: bootcnf/bootcnf cfg_testingtool/pspbtcnf_dc.txt TM/DC10
	bootcnf/bootcnf -it cfg_testingtool/pspbtcnf_dc.txt -ob pspbtcnf_dc.bin
	psptools/pack_bootcfg.py pspbtcnf_dc.bin TM/DC10/testingtool/kd/pspbtcnf_dc.bin
	rm pspbtcnf_dc.bin

TM/DC10/testingtool/kd/pspbtcnf_umd.bin: bootcnf/bootcnf cfg_testingtool/pspbtcnf_umd.txt TM/DC10
	bootcnf/bootcnf -it cfg_testingtool/pspbtcnf_umd.txt -ob pspbtcnf_umd.bin
	psptools/pack_bootcfg.py pspbtcnf_umd.bin TM/DC10/testingtool/kd/pspbtcnf_umd.bin
	rm pspbtcnf_umd.bin

TM/DC10/testingtool/kd/pspbtcnf_np.bin: bootcnf/bootcnf cfg_testingtool/pspbtcnf_np.txt TM/DC10
	bootcnf/bootcnf -it cfg_testingtool/pspbtcnf_np.txt -ob pspbtcnf_np.bin
	psptools/pack_bootcfg.py pspbtcnf_np.bin TM/DC10/testingtool/kd/pspbtcnf_np.bin
	rm pspbtcnf_np.bin

TM/DC10/testingtool/kd/pspbtcnf_m33.bin: bootcnf/bootcnf cfg_testingtool/pspbtcnf_m33.txt TM/DC10
	bootcnf/bootcnf -it cfg_testingtool/pspbtcnf_m33.txt -ob pspbtcnf_m33.bin
	psptools/pack_bootcfg.py pspbtcnf_m33.bin TM/DC10/testingtool/kd/pspbtcnf_m33.bin
	rm pspbtcnf_m33.bin

TM/DC10/testingtool/kd/pspbtcnf_recovery.bin: bootcnf/bootcnf cfg_testingtool/pspbtcnf_recovery.txt TM/DC10
	bootcnf/bootcnf -it cfg_testingtool/pspbtcnf_recovery.txt -ob pspbtcnf_recovery.bin
	psptools/pack_bootcfg.py pspbtcnf_recovery.bin TM/DC10/testingtool/kd/pspbtcnf_recovery.bin
	rm pspbtcnf_recovery.bin

TM/DC10/testingtool/kd/pspbtcnf_02g_dc.bin: bootcnf/bootcnf cfg_testingtool/pspbtcnf_02g_dc.txt TM/DC10
	bootcnf/bootcnf -it cfg_testingtool/pspbtcnf_02g_dc.txt -ob pspbtcnf_02g_dc.bin
	psptools/pack_bootcfg.py pspbtcnf_02g_dc.bin TM/DC10/testingtool/kd/pspbtcnf_02g_dc.bin
	rm pspbtcnf_02g_dc.bin

TM/DC10/testingtool/kd/pspbtcnf_02g_umd.bin: bootcnf/bootcnf cfg_testingtool/pspbtcnf_02g_umd.txt TM/DC10
	bootcnf/bootcnf -it cfg_testingtool/pspbtcnf_02g_umd.txt -ob pspbtcnf_02g_umd.bin
	psptools/pack_bootcfg.py pspbtcnf_02g_umd.bin TM/DC10/testingtool/kd/pspbtcnf_02g_umd.bin
	rm pspbtcnf_02g_umd.bin

TM/DC10/testingtool/kd/pspbtcnf_02g_np.bin: bootcnf/bootcnf cfg_testingtool/pspbtcnf_02g_np.txt TM/DC10
	bootcnf/bootcnf -it cfg_testingtool/pspbtcnf_02g_np.txt -ob pspbtcnf_02g_np.bin
	psptools/pack_bootcfg.py pspbtcnf_02g_np.bin TM/DC10/testingtool/kd/pspbtcnf_02g_np.bin
	rm pspbtcnf_02g_np.bin

TM/DC10/testingtool/kd/pspbtcnf_02g_m33.bin: bootcnf/bootcnf cfg_testingtool/pspbtcnf_02g_m33.txt TM/DC10
	bootcnf/bootcnf -it cfg_testingtool/pspbtcnf_02g_m33.txt -ob pspbtcnf_02g_m33.bin
	psptools/pack_bootcfg.py pspbtcnf_02g_m33.bin TM/DC10/testingtool/kd/pspbtcnf_02g_m33.bin
	rm pspbtcnf_02g_m33.bin

TM/DC10/testingtool/kd/pspbtcnf_02g_recovery.bin: bootcnf/bootcnf cfg_testingtool/pspbtcnf_02g_recovery.txt TM/DC10
	bootcnf/bootcnf -it cfg_testingtool/pspbtcnf_02g_recovery.txt -ob pspbtcnf_02g_recovery.bin
	psptools/pack_bootcfg.py pspbtcnf_02g_recovery.bin TM/DC10/testingtool/kd/pspbtcnf_02g_recovery.bin
	rm pspbtcnf_02g_recovery.bin

TM/DC10/testingtool/kd/pspbtcnf_03g_dc.bin: bootcnf/bootcnf cfg_testingtool/pspbtcnf_03g_dc.txt TM/DC10
	bootcnf/bootcnf -it cfg_testingtool/pspbtcnf_03g_dc.txt -ob pspbtcnf_03g_dc.bin
	psptools/pack_bootcfg.py pspbtcnf_03g_dc.bin TM/DC10/testingtool/kd/pspbtcnf_03g_dc.bin
	rm pspbtcnf_03g_dc.bin

TM/DC10/testingtool/kd/pspbtcnf_03g_umd.bin: bootcnf/bootcnf cfg_testingtool/pspbtcnf_03g_umd.txt TM/DC10
	bootcnf/bootcnf -it cfg_testingtool/pspbtcnf_03g_umd.txt -ob pspbtcnf_03g_umd.bin
	psptools/pack_bootcfg.py pspbtcnf_03g_umd.bin TM/DC10/testingtool/kd/pspbtcnf_03g_umd.bin
	rm pspbtcnf_03g_umd.bin

TM/DC10/testingtool/kd/pspbtcnf_03g_np.bin: bootcnf/bootcnf cfg_testingtool/pspbtcnf_03g_np.txt TM/DC10
	bootcnf/bootcnf -it cfg_testingtool/pspbtcnf_03g_np.txt -ob pspbtcnf_03g_np.bin
	psptools/pack_bootcfg.py pspbtcnf_03g_np.bin TM/DC10/testingtool/kd/pspbtcnf_03g_np.bin
	rm pspbtcnf_03g_np.bin

TM/DC10/testingtool/kd/pspbtcnf_03g_m33.bin: bootcnf/bootcnf cfg_testingtool/pspbtcnf_03g_m33.txt TM/DC10
	bootcnf/bootcnf -it cfg_testingtool/pspbtcnf_03g_m33.txt -ob pspbtcnf_03g_m33.bin
	psptools/pack_bootcfg.py pspbtcnf_03g_m33.bin TM/DC10/testingtool/kd/pspbtcnf_03g_m33.bin
	rm pspbtcnf_03g_m33.bin

TM/DC10/testingtool/kd/pspbtcnf_03g_recovery.bin: bootcnf/bootcnf cfg_testingtool/pspbtcnf_03g_recovery.txt TM/DC10
	bootcnf/bootcnf -it cfg_testingtool/pspbtcnf_03g_recovery.txt -ob pspbtcnf_03g_recovery.bin
	psptools/pack_bootcfg.py pspbtcnf_03g_recovery.bin TM/DC10/testingtool/kd/pspbtcnf_03g_recovery.bin
	rm pspbtcnf_03g_recovery.bin

TM/DC10/tmctrl.prx: TM/DC10 ipl/common/libiplsdk.a
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
	python3 psptools/pack_module.py modules/flashemu/flashemu.prx TM/DC10/tmctrl.prx --tag 0x4c9416f0
	
TM/DC10/kd/ipl_update.prx: TM/DC10
	make -C modules/ipl_update
	python3 psptools/pack_module.py modules/ipl_update/ipl_update.prx TM/DC10/kd/ipl_update.prx --tag 0x4c9416f0

TM/DC10/kd/resurrection.prx: TM/DC10
	make -C modules/vunbricker
	python3 psptools/pack_module.py modules/vunbricker/resurrection.prx TM/DC10/kd/resurrection.prx --tag 0x38020af0

TM/DC10/kd/dcman.prx: TM/DC10
	make -C modules/dcman
	python3 psptools/pack_module.py modules/dcman/dcman.prx TM/DC10/kd/dcman.prx --tag 0x4c9416f0

TM/DC10/kd/iop.prx: TM/DC10
	make -C modules/iop
	python3 psptools/pack_module.py modules/iop/iop.prx TM/DC10/kd/iop.prx --tag 0x4c9416f0

TM/DC10/kd/lflash_fdisk.prx: TM/DC10
	make -C modules/lflash_fdisk
	python3 psptools/pack_module.py modules/lflash_fdisk/lflash_fdisk.prx TM/DC10/kd/lflash_fdisk.prx --tag 0x4c9416f0

TM/DC10/kd/idsregeneration.prx: TM/DC10
	make -C modules/idsregeneration
	python3 psptools/pack_module.py modules/idsregeneration/idsregeneration.prx TM/DC10/kd/idsregeneration.prx --tag 0x4c9416f0
	
TM/DC10/kd/emc_sm_updater.prx: TM/DC10
	python3 psptools/pack_module.py modules/emc_sm_updater.prx TM/DC10/kd/emc_sm_updater.prx --tag 0x4c9416f0

TM/DC10/kd/lfatfs_updater.prx: TM/DC10
	python3 psptools/pack_module.py modules/lfatfs_updater.prx TM/DC10/kd/lfatfs_updater.prx --tag 0x4c9416f0

TM/DC10/kd/lflash_fatfmt_updater.prx: TM/DC10
	python3 psptools/pack_module.py modules/lflash_fatfmt_updater.prx TM/DC10/kd/lflash_fatfmt_updater.prx --tag 0x4c9416f0

TM/DC10/vsh/module/intraFont.prx: TM/DC10
	python3 psptools/pack_module.py modules/intraFont.elf TM/DC10/vsh/module/intraFont.prx --tag 0x457b0af0

TM/DC10/vsh/module/vlf.prx: TM/DC10
	python3 psptools/pack_module.py modules/vlf.elf TM/DC10/vsh/module/vlf.prx --tag 0x457b0af0

TM/DC10/kd/pspdecrypt.prx: TM/DC10
	python3 psptools/pack_module.py modules/pspdecrypt.elf TM/DC10/kd/pspdecrypt.prx --tag 0x4c9416f0

TM/DC10/kd/galaxy.prx: TM/DC10
	make -C modules/galaxy
	python3 psptools/pack_module.py modules/galaxy/galaxy.prx TM/DC10/kd/galaxy.prx --tag 0x4c9416f0

TM/DC10/kd/idcanager.prx: TM/DC10
	make -C modules/idcanager
	python3 psptools/pack_module.py modules/idcanager/idcanager.prx TM/DC10/kd/idcanager.prx --tag 0x4c9416f0
	
TM/DC10/kd/march33.prx: TM/DC10
	python3 psptools/pack_module.py modules/march33.elf TM/DC10/kd/march33.prx --tag 0x4c9416f0
	
TM/DC10/kd/popcorn.prx: TM/DC10
	make -C modules/popcorn
	python3 psptools/pack_module.py modules/popcorn/popcorn.prx TM/DC10/kd/popcorn.prx --tag 0x4c9416f0

TM/DC10/kd/nidresolver.prx: TM/DC10
	make -C modules/nidresolver
	python3 psptools/pack_module.py modules/nidresolver/nidresolver.prx TM/DC10/kd/nidresolver.prx --tag 0x4c9416f0

TM/DC10/kd/systemctrl.prx: TM/DC10 ipl/common/libiplsdk.a
	make -C ipl/rebootex clean
	make -C ipl/rebootex BFLAGS="-DIPL_01G"
	cat ipl/rebootex/rebootex.bin | gzip > rebootex.bin.gz
	$(PSPDEV)/bin/bin2s rebootex.bin.gz modules/systemctrl/rebootex.S rebootex
	rm rebootex.bin.gz
	
	make -C ipl/rebootex clean
	make -C ipl/rebootex BFLAGS="-DIPL_02G"
	cat ipl/rebootex/rebootex.bin | gzip > rebootex.bin.gz
	$(PSPDEV)/bin/bin2s rebootex.bin.gz modules/systemctrl/rebootex_02g.S rebootex_02g
	rm rebootex.bin.gz
	
	make -C ipl/rebootex clean
	make -C ipl/rebootex BFLAGS="-DIPL_03G"
	cat ipl/rebootex/rebootex.bin | gzip > rebootex.bin.gz
	$(PSPDEV)/bin/bin2s rebootex.bin.gz modules/systemctrl/rebootex_03g.S rebootex_03g
	rm rebootex.bin.gz
	
	make -C modules/systemctrl
	python3 psptools/pack_module.py modules/systemctrl/systemctrl.prx TM/DC10/kd/systemctrl.prx --tag 0x4c9416f0

TM/DC10/kd/usbdevice.prx: TM/DC10
	make -C modules/usbdevice
	python3 psptools/pack_module.py modules/usbdevice/usbdevice.prx TM/DC10/kd/usbdevice.prx --tag 0x4c9416f0

TM/DC10/kd/vshctrl.prx: TM/DC10
	make -C modules/vshctrl
	python3 psptools/pack_module.py modules/vshctrl/vshctrl.prx TM/DC10/kd/vshctrl.prx --tag 0x4c9416f0

TM/DC10/vsh/module/recovery.prx: TM/DC10
	python3 psptools/pack_module.py modules/recovery.elf TM/DC10/vsh/module/recovery.prx --tag 0x38020af0

TM/DC10/vsh/module/satelite.prx: TM/DC10
	make -C modules/satelite
	python3 psptools/pack_module.py modules/satelite/satelite.prx TM/DC10/vsh/module/satelite.prx --tag 0x457b0af0


