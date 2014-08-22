#!/bin/sh

g++ -g -oecho_server echo_server.cpp -I../include  ../src/CEvent.cpp ../src/CCommMgr.cpp ../src/CTask.cpp ../src/CThread.cpp ../src/Utils.cpp  -lpthread 

