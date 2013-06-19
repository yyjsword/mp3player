#written by yyj, 2008-9-17
HEADPATH=../src/inc/
LIBRPATH="../lib"
LKFLAGS = $(LIBRPATH)/winmm.lib $(LIBRPATH)/LIBCMT.LIB $(LIBRPATH)/UUID.LIB $(LIBRPATH)/dec_frame.lib\
                   $(LIBRPATH)/libcpmt.lib $(LIBRPATH)/oldnames.lib $(LIBRPATH)/kernel32.lib $(LIBRPATH)/kernel32.lib
OBJPATH = ../src/obj
OBJPATH2 = ..\src\obj
OBJ = $(OBJPATH)/mp.obj $(OBJPATH)/frameread.obj $(OBJPATH)/wavplay.obj 
EXEPATH=..


#############################################################################
#allsrcpath = $(wildcard *.c )
#allsrcname=$(notdir $(allsrcpath))
#allsrcobj=$(patsubst %.c,%.o,$(allsrcname) )

#	@echo $(allsrcpath)
#	@echo $(allsrcname)
#	@echo $(allsrcobj)
#	@echo "end"
#	$(warning "aaaaaaaa")
#############################################################################



$(EXEPATH)/mp.exe:$(OBJ)
	link /out:$(EXEPATH)/mp.exe $(OBJ) /NODEFAULTLIB:LIBC.LIB $(LKFLAGS) /machine:IX86
$(OBJPATH)/mp.obj:../src/mp.cpp ../src/mp.h
	cl /MT /O2 /G5 -I $(HEADPATH) -c ../src/mp.cpp
	move mp.obj $(OBJPATH)
$(OBJPATH)/frameread.obj:../src/frameread.cpp ../src/dec_frame/frame.h ../src/mp.h
	cl /O2 /G5 -I $(HEADPATH) -c ../src/frameread.cpp
	move frameread.obj $(OBJPATH)
$(OBJPATH)/wavplay.obj:../src/wavplay.cpp ../src/mp.h
	cl /O2 /G5 -I $(HEADPATH) -c ../src/wavplay.cpp
	move wavplay.obj $(OBJPATH)

clean:
	del /q $(EXEPATH)\mp.exe $(OBJPATH2)\mp.obj $(OBJPATH2)\frameread.obj $(OBJPATH2)\wavplay.obj $(OBJPATH2)\layer3.obj $(OBJPATH2)\imdct.obj $(OBJPATH2)\getbits.obj