PROJECT_ROOT = $(dir $(abspath $(lastword $(MAKEFILE_LIST))))


all : exo1 exo2 exo3 exo4 exo5

exo1 : $(PROJECT_ROOT)main.cpp $(PROJECT_ROOT)utils.hh
	g++ -Wall -std=c++1y -pthread -g -O0 $< -o exo1

exo2 : $(PROJECT_ROOT)mainMT.cpp $(PROJECT_ROOT)utils.hh
	g++ -Wall -std=c++1y -pthread -g -O0 $< -o exo2

exo3 : $(PROJECT_ROOT)mainMTfin.cpp $(PROJECT_ROOT)utils.hh
	g++ -Wall -std=c++1y -pthread -g -O0 $< -o exo3

exo4 : $(PROJECT_ROOT)mainContraint.cpp $(PROJECT_ROOT)utils.hh
	g++ -Wall -std=c++1y -pthread -g -O0 $< -o exo4

exo5 : $(PROJECT_ROOT)Result.cpp 
	g++ -Wall -std=c++1y -pthread -g -O0 $< -o exo5


clean :
	\rm exo1
	\rm exo2
	\rm exo3
	\rm exo4
	\rm exo5

