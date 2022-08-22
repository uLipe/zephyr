##############################################################
# Copyright (c) 1986-2022 Xilinx, Inc.  All rights reserved. #
##############################################################

# source /tools/Xilinx/Vitis/2022.1/.settings64-Vitis.sh

export XILINX_VITIS=/tools/Xilinx/Vitis/2022.1
if [ -n "${PATH}" ]; then
  export PATH=/tools/Xilinx/Vitis/2022.1/bin:/tools/Xilinx/Vitis/2022.1/gnu/microblaze/lin/bin:/tools/Xilinx/Vitis/2022.1/gnu/arm/lin/bin:/tools/Xilinx/Vitis/2022.1/gnu/microblaze/linux_toolchain/lin64_le/bin:/tools/Xilinx/Vitis/2022.1/gnu/aarch32/lin/gcc-arm-linux-gnueabi/bin:/tools/Xilinx/Vitis/2022.1/gnu/aarch32/lin/gcc-arm-none-eabi/bin:/tools/Xilinx/Vitis/2022.1/gnu/aarch64/lin/aarch64-linux/bin:/tools/Xilinx/Vitis/2022.1/gnu/aarch64/lin/aarch64-none/bin:/tools/Xilinx/Vitis/2022.1/gnu/armr5/lin/gcc-arm-none-eabi/bin:/tools/Xilinx/Vitis/2022.1/aietools/bin:$PATH
else
  export PATH=/tools/Xilinx/Vitis/2022.1/bin:/tools/Xilinx/Vitis/2022.1/gnu/microblaze/lin/bin:/tools/Xilinx/Vitis/2022.1/gnu/arm/lin/bin:/tools/Xilinx/Vitis/2022.1/gnu/microblaze/linux_toolchain/lin64_le/bin:/tools/Xilinx/Vitis/2022.1/gnu/aarch32/lin/gcc-arm-linux-gnueabi/bin:/tools/Xilinx/Vitis/2022.1/gnu/aarch32/lin/gcc-arm-none-eabi/bin:/tools/Xilinx/Vitis/2022.1/gnu/aarch64/lin/aarch64-linux/bin:/tools/Xilinx/Vitis/2022.1/gnu/aarch64/lin/aarch64-none/bin:/tools/Xilinx/Vitis/2022.1/gnu/armr5/lin/gcc-arm-none-eabi/bin:/tools/Xilinx/Vitis/2022.1/aietools/bin
fi

source /tools/Xilinx/Model_Composer/2022.1/.settings64-Model_Composer.sh
source /tools/Xilinx/Vivado/2022.1/.settings64-Vivado.sh
source /tools/Xilinx/Vitis_HLS/2022.1/.settings64-Vitis_HLS.sh
source /tools/Xilinx/DocNav/.settings64-DocNav.sh
