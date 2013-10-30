#!/bin/sh

g++ -g -otest_log Test_AsyncLog.cpp -I../include ../src/CAsyncLog.cpp ../src/CThread.cpp ../src/Utils.cpp -L./ -ltcmalloc_minimal  -lpthread 

