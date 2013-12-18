#!/bin/sh

g++ -g -otest_comm Test_CCommMgr.cpp -I../include  ../src/CEvent.cpp ../src/CCommMgr.cpp ../src/CTask.cpp ../src/CThread.cpp ../src/Utils.cpp ../src/CHttpResponse.cpp ../src/CHttpParser.cpp  -lpthread 

