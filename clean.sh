rm -rf DC8

cd ipl/msiplldr
make clean
cd -

cd ipl/msipl
make clean
cd -

cd bootcnf
make clean
cd -

cd ipl/payloadex
make clean
cd -

cd ipl/ipl_stage2_payload
make clean
cd -

cd ipl/ipl_stage1_payload
make clean
cd -

cd ipl/rebootex
make clean
cd -

cd modules/flashemu
make clean
cd -

cd modules/ipl_update
make clean
cd -

cd modules/vunbricker
make clean
cd -

cd modules/dcman
make clean
cd -

cd modules/iop
make clean
cd -

cd modules/lflash_fdisk
make clean
cd -

cd modules/idsregeneration
make clean
cd -

cd ipl/nandipl
make clean
cd -

