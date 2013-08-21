#!/bin/sh

g++ -g -otest_lf test_lf.cpp  ../CEvent.cpp ../CCommMgr.cpp ../CTask.cpp ../CThread.cpp ../Utils.cpp ../CHttpResponse.cpp ../CHttpParser.cpp  -lpthread 

