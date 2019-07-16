#!bin/bash
time ./tdm input/SampleInput output/ss
echo "\n [checker]"
./eval/evaluaterFast input/SampleInput output/ss
time ./tdm input/synopsys01.txt output/s1
echo "\n [checker]"
./eval/evaluaterFast input/synopsys01.txt output/s1
time ./tdm input/synopsys02.txt output/s2
echo "\n [checker]"
./eval/evaluaterFast input/synopsys02.txt output/s2
time ./tdm input/synopsys03.txt output/s3
echo "\n [checker]"
./eval/evaluaterFast input/synopsys03.txt output/s3
time ./tdm input/synopsys04.txt output/s4
echo "\n [checker]"
./eval/evaluaterFast input/synopsys04.txt output/s4
time ./tdm input/synopsys05.txt output/s5
echo "\n [checker]"
./eval/evaluaterFast input/synopsys05.txt output/s5
time ./tdm input/synopsys06.txt output/s6
echo "\n [checker]"
./eval/evaluaterFast input/synopsys06.txt output/s6

