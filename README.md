# ICCAD-2019-ProblemB
- System-level FPGA Routing with Timing Division Multiplexing Technique
- [ICCAD Link](http://iccad-contest.org/2019/problems.html)
- language: C++
- compiler: -std=c++11

### Structure
- **src/** : source code directory
- **src/main.cpp**: main 
- **src/component.h & cpp**: data structure for FPGA, nets, ....
- **src/tdm.h & cpp**: for file IO and manage for multiplexing algorithm
- ....
- input/ : released test cases
- output/: output result directory
- evaluater_V3: evaluator

### Compile
```
$ cd src
$ make
$ cd ..
```

### Run 
```
$ ./tdm <input> <output>
```

### Evaluation
```
$ ./evaluater_V3 <input> <output>
```

### Result
https://docs.google.com/spreadsheets/d/16-W2OZZZRlo_uKKbR1mh9Ab0Xff4AxLhppvCP39IYpM/edit?usp=sharing