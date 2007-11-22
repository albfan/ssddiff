LDADD = @libxml2_LIBS@
AM_CXXFLAGS = @libxml2_CFLAGS@

bin_PROGRAMS = xmldiff
xmldiff_SOURCES = diff.cc doc.cc main.cc node.cc node_eqclass.cc out_common.cc out_marked.cc out_merged.cc out_xupdate.cc rel_count.cc rel_eqclass.cc ustring.cc

noinst_HEADERS = config.h diff.h doc.h node_eqclass.h node.h out_common.h out_marked.h out_merged.h out_xupdate.h rel_count.h rel_eqclass.h ustring.h util.h

EXTRA_DIST = COPYING TODO
