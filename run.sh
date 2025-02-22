
cd gnu-efi
make
make bootloader
cd ../kernel
rm -r lib
make kernel
make buildimg
nm -n --defined-only bin/kernel.elf | grep ' T ' | awk '{print $1, $3}' > bin/functions.txt

cp ../ovmf_code.fd ../OVMFbin/ovmf_code.fd
cp ../ovmf_vars.fd ../OVMFbin/ovmf_vars.fd

make run

rm ../OVMFbin/ovmf_code.fd
rm ../OVMFbin/ovmf_vars.fd
cd ../