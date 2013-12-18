#!/bin/sh

g++ -g -otest_server Test_Server.cpp -I../include  ../src/CEvent.cpp ../src/CCommMgr.cpp ../src/CTask.cpp ../src/CThread.cpp ../src/Utils.cpp -lpthread 

