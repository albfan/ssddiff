# ============================================================================
#        Filename:  Makefile
#     Description:  Makefile for SSDDiff application.
# 
#         Version:  0.1
#         Created:  Thu, 15 Jul 2004 14:13:25 +0200
#         Licence:  GPL (read COPYING file for details)
# 
#          Author:  Erich Schubert (eS), erich@debian.org
#                   Institut für Informatik, LMU München
# ============================================================================
CXXFLAGS=-g -Wall -pedantic -DVERBOSE -DCAREFUL -O0
CXXFLAGS_XML=`pkg-config --cflags libxml-2.0`
LDLIBS_XML=`pkg-config --libs libxml-2.0`

.PHONY: all test test2 clean

SRCS_ALL=$(wildcard *.cc)
SRCS_XMLDIFF = \
	doc.cc node.cc node_eqclass.cc rel_count.cc rel_eqclass.cc \
	ustring.cc \
	diff.cc \
	out_common.cc out_marked.cc out_xupdate.cc out_merged.cc \
	main.cc
OBJS_XMLDIFF = $(patsubst %.cc,obj/%.o,$(SRCS_XMLDIFF))

all: warning xmldiff

warning:
	@if [ ! -e obj/.deps ]; then \
		echo "You must run 'make depend' first, then 'make'."; \
		exit 1; \
	fi
	@echo "Don't forget to call 'make depend' if you changed includes."

depend:
	@-mkdir obj
	@touch obj/.depsraw
	@echo "GEN     $@"
	@gccmakedep -f obj/.depsraw -- $(CXXFLAGS) $(CXXFLAGS_XML) -- $(SRCS_ALL)
	@sed -e 's,^\([^ ]*.o\),obj\/\1,' < obj/.depsraw > obj/.deps
	@rm -f obj/.depsraw obj/.depsraw.bak

-include obj/.deps

obj/%.o: %.cc
	@echo "COMPILE $@"
	@$(CXX) -c $(CXXFLAGS) $(CXXFLAGS_XML) $< -o $@

xmldiff: $(OBJS_XMLDIFF)
	@echo "LINK    $@"
	@$(CXX) $(CXXFLAGS) $(CXXFLAGS_XML) $(LDLIBS) $(LDLIBS_XML) $^ -o $@

clean:
	rm -f xmldiff obj/*.o *~
