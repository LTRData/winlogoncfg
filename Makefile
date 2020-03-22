!IF "$(CPU)" == ""
CPU=$(_BUILDARCH)
!ENDIF

!IF "$(CPU)" == ""
CPU=i386
!ENDIF

WARNING_LEVEL=/nologo /WX /W4 /D_CRT_SECURE_NO_WARNINGS /wd4214 /wd4201 /wd4206

!IF "$(DEBUG)" == ""

!IF "$(CPU)" == "i386"

OPTIMIZATION=/nologo /Yc /Ox /GF /GR- /Zi /MD

!ELSE

OPTIMIZATION=/nologo /Yc /Ox /GFS- /GR- /Zi /MD

!ENDIF

!IF "$(CPU)" == "ARM64"

LINK_SWITCHES=/nologo /release /opt:ref,icf=10 /largeaddressaware /debug

!ELSE

LINK_SWITCHES=/nologo /release /opt:nowin98,ref,icf=10 /largeaddressaware /debug

!ENDIF

!ELSE

!IF "$(CPU)" == "i386"

OPTIMIZATION=/nologo /Zi /MD

!ELSE

OPTIMIZATION=/nologo /Zi /MD

!ENDIF

!IF "$(CPU)" == "ARM64"

LINK_SWITCHES=/nologo /release /opt:ref,icf=10 /largeaddressaware /debug /defaultlib:bufferoverflowU.lib

!ELSE

LINK_SWITCHES=/nologo /release /opt:nowin98,ref,icf=10 /largeaddressaware /debug /defaultlib:bufferoverflowU.lib

!ENDIF

!ENDIF

$(CPU)\winlogoncfg.exe: $(CPU)\winlogoncfg.obj winlogoncfg.res winlogoncfg.rc.h ..\lib\minwcrt.lib
	link $(LINK_SWITCHES) /out:$(CPU)\winlogoncfg.exe $(CPU)\winlogoncfg.obj winlogoncfg.res

$(CPU)\winlogoncfg.obj: winlogoncfg.cpp winlogoncfg.rc.h ..\include\winstrct.h Makefile
	cl /c $(WARNING_LEVEL) $(OPTIMIZATION) $(CPP_DEFINE) /Fo$(CPU)\winlogoncfg /Fp$(CPU)\winlogoncfg winlogoncfg.cpp

winlogoncfg.res: winlogoncfg.rc winlogoncfg.rc.h winlogoncfg.ico Makefile
	rc winlogoncfg.rc

install: p:\utils\winlogoncfg.exe p:\utils\winlogoncfg.hlp p:\utils\winlogoncfg.cnt

p:\utils\winlogoncfg.exe: i386\winlogoncfg.exe
	xcopy /d/y winlogoncfg.exe p:\utils\

p:\utils\winlogoncfg.hlp: winlogoncfg.hlp
	xcopy /d/y winlogoncfg.hlp p:\utils\

p:\utils\winlogoncfg.cnt: winlogoncfg.cnt
	xcopy /d/y winlogoncfg.cnt p:\utils\

clean:
	del /s *~ *.obj *.res *.pch *.ph
