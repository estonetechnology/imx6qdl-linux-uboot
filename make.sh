export ARCH=arm
export CROSS_COMPILE=/opt/gcc-linaro-arm-linux-gnueabihf-4.7-2013.04-20130415_linux/bin/arm-linux-gnueabihf-

make hio-imx6dl-board_spl_defconfig
make 
