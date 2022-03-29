#!/bin/sh

export ARCH=arm64
export CROSS_COMPILE=aarch64-linux-gnu-

export INSTALL_MOD_PATH=/home/cong/linux/kernel_install_path
export INSTALL_HDR_PATH=/home/cong/linux//kernel_install_path
export INSTALL_DTBS_PATH=/home/cong/linux/kernel_install_path
export INSTALL_PATH=/home/cong/linux/kernel_install_path

export OUTPUT_PATH=/home/cong/linux/pi_kernel_output

export KERNEL_SRC=/home/cong/linux/mainline_linux
export UBOOT_PATH=/home/cong/linux/u-boot
export CCPREFIX=$CROSS_COMPILE
export MODULES_TEMP=$INSTALL_MOD_PATH

function dir_clean
{
	cd $OUTPUT_PATH
	rm ./* -rf

	cd $INSTALL_PATH
	rm ./* -rf
}

function raspi_build_kernel
{
	cd $KERNEL_SRC
	make O=$OUTPUT_PATH clean 
	make O=$OUTPUT_PATH distclean
	make O=$OUTPUT_PATH mrproper
	make clean && make mrproper
	dir_clean

	cd $KERNEL_SRC

	# make O=$OUTPUT_PATH bcm2711_defconfig
	make O=$OUTPUT_PATH KCONFIG_ALLCONFIG=../rpi_config alldefconfig
	# make O=$OUTPUT_PATH Image modules dtbs -j12
	# make O=$OUTPUT_PATH install modules_install dtbs_install headers_install
	make O=$OUTPUT_PATH bindeb-pkg INSTALL_MOD_STRIP=1 -j16
	# make O=../pi_kernel_output deb-pkg -j12 
	# cd $INSTALL_PATH
	# ls vmlinuz* | xargs chmod +x
	cd ../
	mv *.deb $INSTALL_PATH
	cp ~/bin/pi_update.sh $INSTALL_PATH/
	tar -czvf upgrade.tar.gz $INSTALL_PATH
}

function raspi_build_uboot
{
	cd $UBOOT_PATH
	make rpi_4_defconfig
	make -j12

	cd tools
}

raspi_build_kernel

