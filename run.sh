
#cd user_prog
#./build.sh
#cd ../
cd gnu-efi
make
make bootloader
cd ../kernel
rm -r lib
make kernel
make buildimg
make run
cd ../