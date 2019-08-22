# ICCAD-2019-ProblemB
- System-level FPGA Routing with Timing Division Multiplexing Technique
- [Contest Link](http://iccad-contest.org/2019/problems.html)
- language: C++
- compiler: -std=c++11
- team: cada0030

### Structure
- **src/** : source code directory
- **src/main.cpp**: main 
- **src/component.h & cpp**: data structure for FPGA, nets, ....
- **src/tdm.h & cpp**: for file IO and manage for multiplexing algorithm
- ....
- input/ : released test cases
- output/: output result directory
- eval/: evaluator

### Compile
```
$ cd src
$ make
$ cd ..
```

### Run 
```
$ ./cada0030 <input> <output>
```

### Evaluation
```
$ ./evaluaterFast <input> <output>
```

### Result
https://docs.google.com/spreadsheets/d/16-W2OZZZRlo_uKKbR1mh9Ab0Xff4AxLhppvCP39IYpM/edit?usp=sharing
