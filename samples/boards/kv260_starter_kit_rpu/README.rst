.. _hello_world:

Hello World on RPU unit of KV620 Starter Kit 
############################################

Overview
********

A simple sample that can be used specific to Xilinx KV260 Starter kit and
prints "Hello World" to the RAM console.

Requirements:
*************

Building Zephyr for ZynqMP has a strong dependency from Xilinx Vitis software 
so user needs to install it first, also this has been tested in Linux only, 
Check this user guide on how to install Vitis in your host machine:

https://xilinx.github.io/Vitis-Tutorials/2021-1/build/html/docs/Getting_Started/Vitis/Part2.html

Check if Vitis was properly installed to the /tools directory before using it, user may need to 
perform the installation as super user.

Building
********

This application can be built and executed on QEMU as follows:

.. zephyr-app-commands::
   :zephyr-app: samples/boards/kv260_starter_kit_rpu
   :host-os: unix
   :board: xilinx_kv260_starter_kit_r5
   :goals: build
   :compact:

Running
*******
This sample offers a set of helpers extracted from Vitis scritps to bypass some of its 
bureucracy to load Zephyr to the RPU, allowing user to use the KV260 starter kit on board
JTAG to load and debug a Zephyr application.

.. code-block:: console
    
    export VITIS_FILES_PATH=your_absolute_working_directory_path/zephyr/samples/boards/kv260_starter_kit_rpu/vitis/ 

Now source the customized version of Vitis software settings, this is necessary to use Zephyr CMake instead of the 
Vitis own copy which does not work for Zephyr due to version mismatch:

.. code-block:: console
    
    source zephyr/samples/boards/kv260_starter_kit_rpu/vitis/settings64.sh 

Now run the XIlinx software console:

.. code-block:: console
    
    xsct

The terminal should looks like below:

.. code-block:: console

    rlwrap: warning: your $TERM is 'xterm-256color' but rlwrap couldn't find it in the terminfo database. Expect some problems.
                                                                                                                                                                                        
    ****** Xilinx Software Commandline Tool (XSCT) v2022.1.0
      **** SW Build 3524075 on 2022-04-13-17:42:45
        ** Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
CONFIG_RAM_CONSOLE=y

.. code-block:: console

    100%    0MB   0.2MB/s  00:00                                                                                                                                                            
    Setting PC to Program Start Address 0x00000d90
    Successfully downloaded /home/ulipe/linaro_work/build/zephyr/zephyr.elf
    1
    xsct% Info: Breakpoint 1 status:
    target 6: {Address: 0x16c Type: Hardware}
    xsct%   

Now just type inside of xilinx console: 

.. code-block:: console
    
    con

The zephyr app will start to run until hits the Breakpoint in the main:

Sample Output
=============

On the Xilinx console you should see:

.. code-block:: console

    xsct% con                                                                                                                                                                               
    xsct% Info: Cortex-R5 #0 (target 6) Stopped at 0x16c (Breakpoint)                                                                                                                       
    11:     printk("Hello World! %s\n", CONFIG_BOARD);
    xsct% xsct%      

Open another terminal with your serial terminal application, the KV260 Starter
kit offers three serial ports, the first one is directly wired to the RPU, just
connect to it and on the xsct console hit the con command again to run and you will
should see:

.. code-block:: console

    *** Booting Zephyr OS build zephyr-v3.1.0-2489-g76ec2d18a573  ***
    Hello World! board_kv260_starter_kit_r5

