ifndef NAVISERVER
    NAVISERVER  = /usr/local/ns
endif

#
# Module name
#
MOD      =  nsaccess.so

#
# Objects to build.
#
MODOBJS     = nsaccess.o

MODLIBS	 = -lwrap

include  $(NAVISERVER)/include/Makefile.module
