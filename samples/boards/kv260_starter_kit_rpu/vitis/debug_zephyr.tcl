# Usage with xsct:
# 
# source debug_zephyr.tcl
# 
connect -url tcp:127.0.0.1:3121
source /tools/Xilinx/Vitis/2022.1/scripts/vitis/util/zynqmp_utils.tcl
targets -set -nocase -filter {name =~"APU*"}
rst -system
after 3000
targets -set -nocase -filter {name =~"RPU*"}
enable_split_mode
targets -set -filter {jtag_cable_name =~ "Xilinx X-MLCC-01 XFL1IZTOX2KZA" && level==0 && jtag_device_ctx=="jsn-X-MLCC-01-XFL1IZTOX2KZA-04724093-0"}
targets -set -nocase -filter {name =~"APU*"}
loadhw -hw xilinx-kv260-starterkit-2022.1.xsa -mem-ranges [list {0x80000000 0xbfffffff} {0x400000000 0x5ffffffff} {0x1000000000 0x7fffffffff}] -regs
configparams force-mem-access 1
targets -set -nocase -filter {name =~"APU*"}
source psu_init.tcl
psu_init
source /tools/Xilinx/Vitis/2022.1/scripts/vitis/util/fsbl.tcl
after 1000
psu_ps_pl_isolation_removal
after 1000
psu_ps_pl_reset_config
catch {psu_protection}
targets -set -nocase -filter {name =~ "*R5*#0"}
rst -processor
catch {XFsbl_TcmEccInit R5_0}
dow build/zephyr/zephyr.elf
configparams force-mem-access 0
bpadd -addr &main
