######################################################################
#
#  Licensed Materials - Property of IBM
#  
#  Restricted Materials of IBM 
#  
#  IBM Informix Dynamic Server
#  (c) Copyright IBM Corporation 2013 All rights reserverd.
# 
# 
#   Title            : Makefile
#   Description      : Make definition file for TSLoader
# 
######################################################################

PLATFORM=LINUX
#PLATFORM=AIX

VER=15
SHELL=/bin/bash
INCLUDES=-I$(INFORMIXDIR)/incl/dmi
#DEBUG=-cc -g
DEBUG=-O3
CFLAGS=-Wall -D$(PLATFORM) $(INCLUDES)
ESQLFLAGS=$(CFLAGS) -thread 
DIR=$(shell basename $(PWD))

# There is a problem with the explicit check for thread libraries on
# newer Linux versions. The local copy of esql has the fix. 
#ESQL=./esql
ESQL=esql

# These are put in the tar file for distribution as source
TARFILES=main.c,process.c,exp_chk.ec,database.ec,defs.h,sql_defs.h,proto.h,json.c,hash.c,Makefile,esql,usage.c,mi_search.h,mi_hash.c,threads.c,TSLoader.txt,TSLoader.1,TSL_FileLoad


OBJS=main.o hash.o process.o database.o json.o exp_chk.o usage.o mi_hash.o version.o threads.o

THREADTYPE=POSIX

all:	version TSL_Load TSL_Shutdown TSL_Init

TSL_Load:	$(OBJS)
	THREADLIB=${THREADTYPE} $(ESQL) $(ESQLFLAGS) $(DEBUG) $(OBJS) -o TSL_Load

TSL_Shutdown:	TSL_Load
	@rm -f TSL_Shutdown
	ln -s TSL_Load TSL_Shutdown

TSL_Init:	TSL_Load
	@rm -f TSL_Init
	ln -s TSL_Load TSL_Init

database.c:	database.ec defs.h sql_defs.h mi_search.h
	THREADLIB=$(THREADTYPE) $(ESQL) $(ESQLFLAGS) $(INCLUDES) -nowarn -e database.ec

database.o:	database.c
# Remove annoying error when using -Wall 
	@awk 'BEGIN{p=0}/_SD0 = /{print $0; print "_dummy_SD0 (_SD0);";p=1}/#line 1 "database.ec"/{print "void _dummy_SD0(ifx_sqlda_t *_SD0) {return;};"; print $0; p=1}{if (p == 0) print $0; p=0}' database.c > database.c-fix
	@mv database.c-fix database.c
	THREADLIB=$(THREADTYPE) $(ESQL) $(ESQLFLAGS) $(INCLUDES) -c database.c  $(DEBUG)

exp_chk.o:	exp_chk.ec
	THREADLIB=$(THREADTYPE) $(ESQL) $(ESQLFLAGS) $(INCLUDES) -c exp_chk.ec  $(DEBUG)

main.o: main.c defs.h mi_search.h
	THREADLIB=$(THREADTYPE) $(ESQL) $(ESQLFLAGS) $(INCLUDES) -c main.c  $(DEBUG)

usage.o: usage.c description.c defs.h
	THREADLIB=$(THREADTYPE) $(ESQL) $(ESQLFLAGS) $(INCLUDES) -c usage.c  $(DEBUG)

process.o: process.c defs.h mi_search.h
	THREADLIB=$(THREADTYPE) $(ESQL) $(ESQLFLAGS) $(INCLUDES) -c process.c  $(DEBUG)

json.o: json.c defs.h
	THREADLIB=$(THREADTYPE) $(ESQL) $(ESQLFLAGS) $(INCLUDES) -c json.c  $(DEBUG)

hash.o: hash.c defs.h
	THREADLIB=$(THREADTYPE) $(ESQL) $(ESQLFLAGS) $(INCLUDES) -c hash.c  $(DEBUG)

threads.o: threads.c defs.h
	THREADLIB=$(THREADTYPE) $(ESQL) $(ESQLFLAGS) $(INCLUDES) -c threads.c  $(DEBUG)

mi_hash.o: mi_hash.c defs.h mi_search.h
	THREADLIB=$(THREADTYPE) $(ESQL) $(ESQLFLAGS) $(INCLUDES) -c mi_hash.c $(DEBUG)

version:
	@date '+char TSLoader_ver[] = "TSLoader '$(VER)' %Y-%m-%d %H:%M";' > version.c
	@$(ESQL) -V | awk 'BEGIN{printf "char esql_ver[] = \""} \
		{ printf "%s ", $$0 } END { print "\";" }' >> version.c
	@uname -a  | awk 'BEGIN{printf "char uname_ver[] = \""} \
		{ printf "%s ", $$0 } END { print "\";" }' >> version.c

version.c: version

version.o: version.c
	@echo "char args_ver[] = \"THREADLIB=$(THREADTYPE) $(ESQL) $(ESQLFLAGS) $(INCLUDES) -c version.c $(DEBUG)\";" >>version.c
	THREADLIB=$(THREADTYPE) $(ESQL) $(ESQLFLAGS) $(INCLUDES) -c version.c $(DEBUG) version.c

clean:
	rm -f database.c exp_chk.c TSL_Load TSL_Init TSL_Shutdown *.o 

tar:
	(cd ..; tar cvfz ${DIR}/cTSLoader.tgz ${DIR}/{${TARFILES}})

man:	TSLoader.1

description.c:	TSLoader.txt
	sed -e 's/%/%%/g' TSLoader.txt | awk 'BEGIN{print "/* Auto generated from TSLoader.txt */"}{printf "printf (\"%s\\n\");\n", $$0}' > description.c

TSLoader.1:	TSL_Init
	./TSL_Init --man | txt2man -t 'TSLoader' -r 'Informix TimeSeries' > TSLoader.1

testcompile:	all tar
	rm -rf test
	mkdir -p test
	(cd test ; tar xvf ../cTSLoader.tgz ; cd cTSLoader;	make)

save:	tar
	mv cTSLoader.tgz Versions/cTSLoader.tgz-$(VER)
