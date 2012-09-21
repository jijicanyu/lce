#!/bin/sh

g++ -otest_comm Test_CCommMgr.cpp  ../CEvent.cpp ../CCommMgr.cpp ../CTask.cpp ../CThread.cpp ../Utils.cpp ../CHttpResponse.cpp ../CHttpParser.cpp  -lpthread 

