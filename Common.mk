##   ___________		     _________		      _____  __
##   \_	  _____/______   ____   ____ \_   ___ \____________ _/ ____\/  |_
##    |    __) \_  __ \_/ __ \_/ __ \/    \  \/\_  __ \__  \\   __\\   __\ 
##    |     \   |  | \/\  ___/\  ___/\     \____|  | \// __ \|  |   |  |
##    \___  /   |__|    \___  >\___  >\______  /|__|  (____  /__|   |__|
##	  \/		    \/	   \/	     \/		   \/
##  ______________________                           ______________________
##			  T H E   W A R   B E G I N S
##	   FreeCraft - A free fantasy real time strategy game engine
##
##	Common.mk        -       Common make (GNU Make)
##
##	(c) Copyright 1998,2000 by Lutz Sammer
##
##	$Id$
##

all:	$(OBJS)

doc:	$(OBJS:.o=.doc) $(HDRS:.h=.doc)

clean::
	$(RM) $(OBJS) core *.doc

clobber: clean
	$(RM) .depend .#* *~ *.o

depend::
	@echo -n >.depend
	@for i in $(OBJS:.o=.c) ; do\
	$(CC) -MM $(IFLAGS) $(DFLAGS) $$i >>.depend ; done

tags::
	@for i in $(OBJS:.o=.c) ; do\
	cd .. ;\
	ctags $(CTAGSFLAGS) $(TAGS) $(MODULE)/$$i ;\
	cd $(MODULE) ;\
	done

ci::
	ci -l $(OBJS:.o=.c) $(HDRS) Makefile

lockver::
	$(LOCKVER) $(OBJS:.o=.c) $(HDRS) Makefile

distlist::
	@echo >>$(DISTLIST)
	@for i in `echo $(OBJS:.o=.c)` $(HDRS) Makefile $(EXTRA) ; do \
	echo src/$(MODULE)/$$i >>$(DISTLIST) ; done

$(OBJS): $(TOPDIR)/Rules.make

#
#	include dependency files, if they exist
#
ifeq (.depend,$(wildcard .depend))
include .depend
endif
