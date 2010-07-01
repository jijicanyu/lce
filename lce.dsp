# Microsoft Developer Studio Project File - Name="lce" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=lce - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "lce.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "lce.mak" CFG="lce - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "lce - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "lce - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "lce - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD BASE RSC /l 0x804 /d "NDEBUG"
# ADD RSC /l 0x804 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib  kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib  kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386

!ELSEIF  "$(CFG)" == "lce - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ  /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ  /c
# ADD BASE RSC /l 0x804 /d "_DEBUG"
# ADD RSC /l 0x804 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib  kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib  kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "lce - Win32 Release"
# Name "lce - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\SRC\CAsyncLog.cpp
# End Source File
# Begin Source File

SOURCE=.\SRC\CCommMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\SRC\CConfig.cpp
# End Source File
# Begin Source File

SOURCE=.\SRC\CEvent.cpp
# End Source File
# Begin Source File

SOURCE=.\SRC\CFileLog.cpp
# End Source File
# Begin Source File

SOURCE=.\SRC\CHttpParser.cpp
# End Source File
# Begin Source File

SOURCE=.\SRC\CHttpResponse.cpp
# End Source File
# Begin Source File

SOURCE=.\SRC\CLog.cpp
# End Source File
# Begin Source File

SOURCE=.\SRC\CNetWorker.cpp
# End Source File
# Begin Source File

SOURCE=.\SRC\CTask.cpp
# End Source File
# Begin Source File

SOURCE=.\SRC\CThread.cpp
# End Source File
# Begin Source File

SOURCE=.\SRC\Utils.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\INCLUDE\CAnyValue.h
# End Source File
# Begin Source File

SOURCE=.\INCLUDE\CAsyncLog.h
# End Source File
# Begin Source File

SOURCE=.\INCLUDE\CBitMap.h
# End Source File
# Begin Source File

SOURCE=.\INCLUDE\CCommMgr.h
# End Source File
# Begin Source File

SOURCE=.\INCLUDE\CConfig.h
# End Source File
# Begin Source File

SOURCE=.\INCLUDE\CEvent.h
# End Source File
# Begin Source File

SOURCE=.\INCLUDE\CFileLog.h
# End Source File
# Begin Source File

SOURCE=.\INCLUDE\CH2ShortT3PackageFilter.h
# End Source File
# Begin Source File

SOURCE=.\INCLUDE\CH2T3PackageFilter.h
# End Source File
# Begin Source File

SOURCE=.\INCLUDE\CHttpPackageFilter.h
# End Source File
# Begin Source File

SOURCE=.\INCLUDE\CHttpParser.h
# End Source File
# Begin Source File

SOURCE=.\INCLUDE\CHttpResponse.h
# End Source File
# Begin Source File

SOURCE=.\INCLUDE\CLock.h
# End Source File
# Begin Source File

SOURCE=.\INCLUDE\CLog.h
# End Source File
# Begin Source File

SOURCE=.\INCLUDE\CLruCache.h
# End Source File
# Begin Source File

SOURCE=.\INCLUDE\CNetWorker.h
# End Source File
# Begin Source File

SOURCE=.\INCLUDE\CNetWorkerMgr.h
# End Source File
# Begin Source File

SOURCE=.\INCLUDE\CPackage.h
# End Source File
# Begin Source File

SOURCE=.\INCLUDE\CPackageFilter.h
# End Source File
# Begin Source File

SOURCE=.\INCLUDE\CProcessor.h
# End Source File
# Begin Source File

SOURCE=.\INCLUDE\CRawPackageFilter.h
# End Source File
# Begin Source File

SOURCE=.\INCLUDE\CRegExp.h
# End Source File
# Begin Source File

SOURCE=.\INCLUDE\CShm.h
# End Source File
# Begin Source File

SOURCE=.\INCLUDE\CShmArray.h
# End Source File
# Begin Source File

SOURCE=.\INCLUDE\CSingleton.h
# End Source File
# Begin Source File

SOURCE=.\INCLUDE\CSocketBuf.h
# End Source File
# Begin Source File

SOURCE=.\INCLUDE\CTask.h
# End Source File
# Begin Source File

SOURCE=.\INCLUDE\CThread.h
# End Source File
# Begin Source File

SOURCE=.\INCLUDE\lce.h
# End Source File
# Begin Source File

SOURCE=.\INCLUDE\StringHelper.h
# End Source File
# Begin Source File

SOURCE=.\INCLUDE\Utils.h
# End Source File
# End Group
# Begin Group "test"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\TEST\Press_CAnyValue.cpp
# End Source File
# Begin Source File

SOURCE=.\TEST\Test_AsyncLog.cpp
# End Source File
# Begin Source File

SOURCE=.\TEST\Test_BitMap.cpp
# End Source File
# Begin Source File

SOURCE=.\TEST\Test_CAnyValue.cpp
# End Source File
# Begin Source File

SOURCE=.\TEST\Test_CCommMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\TEST\Test_CEvent.cpp
# End Source File
# Begin Source File

SOURCE=.\TEST\Test_CNetWorker.cpp
# End Source File
# Begin Source File

SOURCE=.\TEST\Test_Conn.cpp
# End Source File
# Begin Source File

SOURCE=.\TEST\Test_Connector.cpp
# End Source File
# Begin Source File

SOURCE=.\TEST\Test_CTask.cpp
# End Source File
# Begin Source File

SOURCE=.\TEST\Test_CThread.cpp
# End Source File
# Begin Source File

SOURCE=.\TEST\test_hashmap.cpp
# End Source File
# Begin Source File

SOURCE=.\TEST\Test_JsonToAnyValue.cpp
# End Source File
# Begin Source File

SOURCE=.\TEST\test_lf.cpp
# End Source File
# Begin Source File

SOURCE=.\TEST\Test_Server.cpp
# End Source File
# Begin Source File

SOURCE=.\TEST\Test_shm.cpp
# End Source File
# Begin Source File

SOURCE=.\TEST\Test_shm2.cpp
# End Source File
# Begin Source File

SOURCE=.\TEST\Test_StringHelper.cpp
# End Source File
# Begin Source File

SOURCE=.\TEST\Test_Template.cpp
# End Source File
# Begin Source File

SOURCE=.\TEST\Test_time.cpp
# End Source File
# Begin Source File

SOURCE=.\TEST\test_unordered_map.cpp
# End Source File
# End Group
# End Target
# End Project
