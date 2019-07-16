#!bin/bash
time ./tdm input/SampleInput output/ss
echo ""
echo " [checker]"
./eval/evaluaterFast input/SampleInput output/ss
for i in {1..6};
do
    time ./tdm input/synopsys0${i}.txt output/s${i}
    echo ""
    echo " [checker]"
    ./eval/evaluaterFast input/synopsys0${i}.txt output/s${i}
done