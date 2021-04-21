rm -rf DC8
mkdir DC8
cp -R 500/flash0/* DC8/
cp -R prebuilt/* DC8/
mkdir -p DC8/dic
mkdir -p DC8/gps
mkdir -p DC8/net/http
mkdir -p DC8/registry
mkdir -p DC8/vsh/theme

cd ipl/msiplldr
make clean
make
cd -

psptools/pack_ipl.py ipl/msiplldr/msiplldr.bin DC8/msipl.bin

cd ipl/msipl
make clean
make
cd -

dd if=ipl/msipl/msipl.bin of=DC8/msipl.bin bs=1 seek=4096

cd bootcnf
make
cd -

./bootcnf/bootcnf -it cfg/pspbtdnf.txt -ob DC8/kd/pspbtdnf.bin
./bootcnf/bootcnf -it cfg/pspbtjnf.txt -ob DC8/kd/pspbtjnf.bin
./bootcnf/bootcnf -it cfg/pspbtknf.txt -ob DC8/kd/pspbtknf.bin
./bootcnf/bootcnf -it cfg/pspbtlnf.txt -ob DC8/kd/pspbtlnf.bin
./bootcnf/bootcnf -it cfg/pspbtrnf.txt -ob DC8/kd/pspbtrnf.bin

./bootcnf/bootcnf -it cfg/pspbtdnf_02g.txt -ob DC8/kd/pspbtdnf_02g.bin
./bootcnf/bootcnf -it cfg/pspbtjnf_02g.txt -ob DC8/kd/pspbtjnf_02g.bin
./bootcnf/bootcnf -it cfg/pspbtknf_02g.txt -ob DC8/kd/pspbtknf_02g.bin
./bootcnf/bootcnf -it cfg/pspbtlnf_02g.txt -ob DC8/kd/pspbtlnf_02g.bin
./bootcnf/bootcnf -it cfg/pspbtrnf_02g.txt -ob DC8/kd/pspbtrnf_02g.bin

./bootcnf/bootcnf -it cfg/pspbtdnf_03g.txt -ob DC8/kd/pspbtdnf_03g.bin
./bootcnf/bootcnf -it cfg/pspbtjnf_03g.txt -ob DC8/kd/pspbtjnf_03g.bin
./bootcnf/bootcnf -it cfg/pspbtknf_03g.txt -ob DC8/kd/pspbtknf_03g.bin
./bootcnf/bootcnf -it cfg/pspbtlnf_03g.txt -ob DC8/kd/pspbtlnf_03g.bin
./bootcnf/bootcnf -it cfg/pspbtrnf_03g.txt -ob DC8/kd/pspbtrnf_03g.bin

# 01g
cd ipl/payloadex
make clean
make BFLAGS="-DIPL_01G -DMSIPL=1"
cd -
bin2s ipl/payloadex/payloadex.bin ipl/ipl_stage2_payload/payloadex.s payloadex

cd ipl/ipl_stage2_payload
make clean
make CFLAGS="-DIPL_01G -DMSIPL=1"
cd -
bin2s ipl/ipl_stage2_payload/ipl_stage2_payload.bin ipl/ipl_stage1_payload/payload.s payload

cd ipl/ipl_stage1_payload
make clean
make CFLAGS="-DIPL_01G -DMSIPL=1"
cd -

dd if=ipl/ipl_stage1_payload/ipl_stage1_payload.bin of=DC8/ipl_01g.bin
dd if=500/ipl_01g.bin of=DC8/ipl_01g.bin bs=1 seek=12288

# 02g
cd ipl/payloadex
make clean
make BFLAGS="-DIPL_02G -DMSIPL=1"
cd -
bin2s ipl/payloadex/payloadex.bin ipl/ipl_stage2_payload/payloadex.s payloadex

cd ipl/ipl_stage2_payload
make clean
make CFLAGS="-DIPL_02G -DMSIPL=1"
cd -
bin2s ipl/ipl_stage2_payload/ipl_stage2_payload.bin ipl/ipl_stage1_payload/payload.s payload

cd ipl/ipl_stage1_payload
make clean
make CFLAGS="-DIPL_02G -DMSIPL=1"
cd -

dd if=ipl/ipl_stage1_payload/ipl_stage1_payload.bin of=DC8/ipl_02g.bin
dd if=500/ipl_02g.bin of=DC8/ipl_02g.bin bs=1 seek=12288

# 03g
cd ipl/payloadex
make clean
make BFLAGS="-DIPL_03G -DMSIPL=1"
cd -
bin2s ipl/payloadex/payloadex.bin ipl/ipl_stage2_payload/payloadex.s payloadex

cd ipl/ipl_stage2_payload
make clean
make CFLAGS="-DIPL_03G -DMSIPL=1"
cd -
bin2s ipl/ipl_stage2_payload/ipl_stage2_payload.bin ipl/ipl_stage1_payload/payload.s payload

cd ipl/ipl_stage1_payload
make clean
make CFLAGS="-DIPL_03G -DMSIPL=1"
cd -

dd if=ipl/ipl_stage1_payload/ipl_stage1_payload.bin of=DC8/ipl_03g.bin
dd if=500/ipl_03g.bin of=DC8/ipl_03g.bin bs=1 seek=12288

# build flashemu
# 01g
cd ipl/rebootex
make clean
make BFLAGS=-DIPL_01G
cd -

cat ipl/rebootex/rebootex.bin | gzip > rebootex.bin.gz
bin2s rebootex.bin.gz modules/flashemu/rebootex.S rebootex
rm rebootex.bin.gz

# 02g
cd ipl/rebootex
make clean
make BFLAGS=-DIPL_02G
cd -

cat ipl/rebootex/rebootex.bin | gzip > rebootex_02g.bin.gz
bin2s rebootex_02g.bin.gz modules/flashemu/rebootex_02g.S rebootex_02g
rm rebootex_02g.bin.gz

# 03g
cd ipl/rebootex
make clean
make BFLAGS=-DIPL_03G
cd -

cat ipl/rebootex/rebootex.bin | gzip > rebootex_03g.bin.gz
bin2s rebootex_03g.bin.gz modules/flashemu/rebootex_03g.S rebootex_03g
rm rebootex_03g.bin.gz

cd modules/flashemu
make
cd -
python3 psptools/pack_kernel_module.py modules/flashemu/flashemu.prx DC8/tmctrl500.prx --tag 0x4c9416f0

cd modules/ipl_update
make
cd -
python3 psptools/pack_kernel_module.py modules/ipl_update/ipl_update.prx DC8/kd/ipl_update.prx --tag 0x4c9416f0

cd modules/vunbricker
make
cd -
python3 psptools/pack_kernel_module.py modules/vunbricker/resurrection.prx DC8/kd/resurrection.prx --tag 0x38020af0

cd modules/dcman
make
cd -
python3 psptools/pack_kernel_module.py modules/dcman/dcman.prx DC8/kd/dcman.prx --tag 0x4c9416f0

cd modules/iop
make
cd -
python3 psptools/pack_kernel_module.py modules/iop/iop.prx DC8/kd/iop.prx --tag 0x4c9416f0

cd modules/lflash_fdisk
make
cd -
python3 psptools/pack_kernel_module.py modules/lflash_fdisk/lflash_fdisk.prx DC8/kd/lflash_fdisk.prx --tag 0x4c9416f0

cd modules/idsregeneration
make
cd -
python3 psptools/pack_kernel_module.py modules/idsregeneration/idsregeneration.prx DC8/kd/idsregeneration.prx --tag 0x4c9416f0

# build nandipl
cd ipl/nandipl
make clean
make
cd -

# 01g
psptools/pack_ipl.py ipl/nandipl/nandipl.bin DC8/nandipl_01g.bin

cd ipl/payloadex
make clean
make BFLAGS="-DIPL_01G"
cd -
bin2s ipl/payloadex/payloadex.bin ipl/ipl_stage2_payload/payloadex.s payloadex

cd ipl/ipl_stage2_payload
make clean
make CFLAGS="-DIPL_01G"
cd -
bin2s ipl/ipl_stage2_payload/ipl_stage2_payload.bin ipl/ipl_stage1_payload/payload.s payload

cd ipl/ipl_stage1_payload
make clean
make CFLAGS="-DIPL_01G"
cd -

dd if=ipl/ipl_stage1_payload/ipl_stage1_payload.bin of=DC8/nandipl_01g.bin bs=1 seek=4096
dd if=500/ipl_01g.bin of=DC8/nandipl_01g.bin bs=1 seek=16384

# 02g
psptools/pack_ipl.py ipl/nandipl/nandipl.bin DC8/nandipl_02g.bin

cd ipl/payloadex
make clean
make BFLAGS="-DIPL_02G"
cd -
bin2s ipl/payloadex/payloadex.bin ipl/ipl_stage2_payload/payloadex.s payloadex

cd ipl/ipl_stage2_payload
make clean
make CFLAGS="-DIPL_02G"
cd -
bin2s ipl/ipl_stage2_payload/ipl_stage2_payload.bin ipl/ipl_stage1_payload/payload.s payload

cd ipl/ipl_stage1_payload
make clean
make CFLAGS="-DIPL_02G"
cd -

dd if=ipl/ipl_stage1_payload/ipl_stage1_payload.bin of=DC8/nandipl_02g.bin bs=1 seek=4096
dd if=500/ipl_02g.bin of=DC8/nandipl_02g.bin bs=1 seek=16384

# 03g
psptools/pack_ipl.py ipl/nandipl/nandipl.bin DC8/nandipl_03g.bin

cd ipl/payloadex
make clean
make BFLAGS="-DIPL_03G"
cd -
bin2s ipl/payloadex/payloadex.bin ipl/ipl_stage2_payload/payloadex.s payloadex

cd ipl/ipl_stage2_payload
make clean
make CFLAGS="-DIPL_03G"
cd -
bin2s ipl/ipl_stage2_payload/ipl_stage2_payload.bin ipl/ipl_stage1_payload/payload.s payload

cd ipl/ipl_stage1_payload
make clean
make CFLAGS="-DIPL_03G"
cd -

dd if=ipl/ipl_stage1_payload/ipl_stage1_payload.bin of=DC8/nandipl_03g.bin bs=1 seek=4096
dd if=500/ipl_03g.bin of=DC8/nandipl_03g.bin bs=1 seek=16384

# sign rest of the modules

# blobs
python3 psptools/pack_kernel_module.py modules/emc_sm_updater.prx DC8/kd/emc_sm_updater.prx --tag 0x4c9416f0
python3 psptools/pack_kernel_module.py modules/lfatfs_updater.prx DC8/kd/lfatfs_updater.prx --tag 0x4c9416f0
python3 psptools/pack_kernel_module.py modules/lflash_fatfmt_updater.prx DC8/kd/lflash_fatfmt_updater.prx --tag 0x4c9416f0
python3 psptools/pack_kernel_module.py modules/intraFont.elf DC8/vsh/module/intraFont.prx --tag 0x457b0af0
python3 psptools/pack_kernel_module.py modules/vlf.elf DC8/vsh/module/vlf.prx --tag 0x457b0af0

# dc
python3 psptools/pack_kernel_module.py modules/pspdecrypt.elf DC8/kd/pspdecrypt.prx --tag 0x4c9416f0

#m33
python3 psptools/pack_kernel_module.py modules/galaxy.elf DC8/kd/galaxy.prx --tag 0x4c9416f0
python3 psptools/pack_kernel_module.py modules/idcanager.elf DC8/kd/idcanager.prx --tag 0x4c9416f0
python3 psptools/pack_kernel_module.py modules/march33.elf DC8/kd/march33.prx --tag 0x4c9416f0
python3 psptools/pack_kernel_module.py modules/popcorn.elf DC8/kd/popcorn.prx --tag 0x4c9416f0
python3 psptools/pack_kernel_module.py modules/systemctrl.elf DC8/kd/systemctrl.prx --tag 0x4c9416f0
python3 psptools/pack_kernel_module.py modules/systemctrl_02g.elf DC8/kd/systemctrl_02g.prx --tag 0x4C9417F0
python3 psptools/pack_kernel_module.py modules/systemctrl_03g.elf DC8/kd/systemctrl_03g.prx --tag 0x4C941FF0
python3 psptools/pack_kernel_module.py modules/usbdevice.elf DC8/kd/usbdevice.prx --tag 0x4c9416f0
python3 psptools/pack_kernel_module.py modules/vshctrl.elf DC8/kd/vshctrl.prx --tag 0x4c9416f0
python3 psptools/pack_kernel_module.py modules/recovery.elf DC8/vsh/module/recovery.prx --tag 0x38020af0
python3 psptools/pack_kernel_module.py modules/satelite.elf DC8/vsh/module/satelite.prx --tag 0x457b0af0

