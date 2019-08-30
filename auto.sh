#!bin/bash
time ./cada0030 input/SampleInput output/ss
echo ""
echo " [checker]"
./eval/evaluaterFast input/SampleInput output/ss
for i in {1..4};
do
    time ./cada0030 input/synopsys0${i}.txt output/s${i}
    echo ""
    echo " [checker]"
    ./eval/evaluaterFast input/synopsys0${i}.txt output/s${i}
done
