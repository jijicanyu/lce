#!/bin/sh

g++ -g -otest_networker -I../include Test_CNetWorker.cpp ../src/CEvent.cpp ../src/CNetWorker.cpp ../src/CTask.cpp ../src/CThread.cpp ../src/Utils.cpp ../src/CHttpResponse.cpp ../src/CHttpParser.cpp  -lpthread 

