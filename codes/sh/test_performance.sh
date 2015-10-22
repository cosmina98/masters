#!/bin/sh

# This script assumes that it's inside the folder /codes/sh/ of a directory
# structure like the one in https://github.com/henriquebecker91/masters.
# Then it proceeds to make the working directory the directory where 
# the c++ code is inside (line bellow).
dir=$(cd -P -- "$(dirname -- "$0")" && pwd -P)
cd $dir
cd ../cpp/

# Before running this script you may want to isolate the CPU that will
# be used (to get more precise times). I (Henrique Becker) do that on
# linux by passing "isolcpus=3" to kernel by the bootloader. I also
# disable the hypertreading (can be done by the BIOS, not in my case,
# or by /sys/devices/system/cpu/cpuX/online, my case). Note that
# the CPU numbering starts at zero, despite what htop tells you.
CPU_FOR_USE=3

# Number of iterations
N=1000

# relative path from where the binaries are to the instances
PATH_I='../../data/ukp/'
# relative path from where the binaries are to where to put the results
PATH_R='../../data/results/pyasukp_benchmark/'

TIME_FORMAT="Ext_time: %e\nExt_mem: %M\n"
CSV_HEADER="UKP5 Internal Time (seconds);UKP5 External Time (seconds);UKP5 Max Memory Use (Kb);Pyasukp Internal Time (seconds);Pyasukp External Time (seconds);Pyasukp Max Memory Use (Kb)"

for f in exnsd16.ukp exnsd18.ukp exnsd20.ukp exnsd26.ukp exnsdbis18.ukp exnsds12.ukp
do
	ukp5_res="$PATH_R${f}5.res"
	pya_res="$PATH_R${f}_pya.res"
	for ((i=0; i < N; ++i))
	do
		taskset -c "$CPU_FOR_USE" time -f "$TIME_FORMAT" ./run_ukp5.out "$PATH_I$f" >> "$ukp5_res" 2>&1
		taskset -c "$CPU_FOR_USE" time -f "$TIME_FORMAT" pyasukp "$PATH_I$f" >> "$pya_res" 2>&1
	done

	grep -E '^Seconds: .*' "$ukp5_res" | cut -d ' ' -f2 > "${ukp5_res}.itime"
	grep -E '^Ext_time: .*' "$ukp5_res" | cut -d ' ' -f2 > "${ukp5_res}.etime"
	grep -E '^Ext_mem: .*' "$ukp5_res" | cut -d ' ' -f2 > "${ukp5_res}.emem"
	paste -d\; "${ukp5_res}.itime" "${ukp5_res}.etime" "${ukp5_res}.emem" > "$ukp5_res"
	rm "${ukp5_res}.itime" "${ukp5_res}.etime" "${ukp5_res}.emem"

	grep -E '^Total Time :.*' "$pya_res" | sed -e 's/Total Time :[[:blank:]]\+//' > "${pya_res}.itime"
	grep -E '^Ext_time: .*' "$pya_res" | cut -d ' ' -f2 > "${pya_res}.etime"
	grep -E '^Ext_mem: .*' "$pya_res" | cut -d ' ' -f2 > "${pya_res}.emem"
	paste -d\; "${pya_res}.itime" "${pya_res}.etime" "${pya_res}.emem" > "$pya_res"
	rm "${pya_res}.itime" "${pya_res}.etime" "${pya_res}.emem"

	echo $CSV_HEADER > "$PATH_R${f}.all"
	paste -d\; "$ukp5_res" "$pya_res" >> "$PATH_R${f}.all"
	rm "$ukp5_res" "$pya_res"
done

echo "If there were no errors the results should be at the $PATH_R folder (from the sh folder)"
