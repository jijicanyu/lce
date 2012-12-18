#!/bin/sh

g++ -g -otest_conn Test_Connector.cpp  ../CEvent.cpp ../CCommMgr.cpp ../CTask.cpp ../CThread.cpp ../Utils.cpp ../CHttpResponse.cpp ../CHttpParser.cpp  -lpthread 

