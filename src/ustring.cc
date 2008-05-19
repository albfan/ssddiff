/* ===========================================================================
 *        Filename:  ustring.cc
 *     Description:  Unified string class
 * 
 *         Version:  $Rev$
 *         Changed:  $Date$
 *         Licence:  GPL (read COPYING file for details)
 * 
 *          Author:  Erich Schubert (eS), erich@debian.org
 *                   Institut für Informatik, LMU München
 * ======================================================================== */

#include "ustring.h"
#include "config.h"

using namespace std;

namespace SSD {

	ustring::ustring(const char* s) {
		const char* string=s;
		if (!s) {
			cstr = NULL;
			return;
		}

#ifndef KEEP_WHITESPACE
		/* skip all whitespace */
		while(isspace(*string)) { string++; }
		/* empty strings are also NULL */
#endif
		if (!*string) {
			cstr = NULL;
			return;
		}

		/* unify text string */
		hashmap< const char*, const char*, hashstr, eq_str>::iterator iter = store.find(string);
		if (iter == store.end()) {
			/* duplicate string */
			cstr = strdup(string);
			/* insert into hashmap */
			store[cstr]=cstr;
		} else
			/* return same string as before */
			cstr = iter->second;
		return;
	}

	ustring::ustring(const xmlChar* s) {
		const char* string=(const char*) s;
		if (!s) {
			cstr = NULL;
			return;
		}

#ifndef KEEP_WHITESPACE
		/* skip all whitespace */
		while(isspace(*string)) { string++; }
		/* empty strings are also NULL */
#endif
		if (!*string) {
			cstr = NULL;
			return;
		}

		/* unify text string */
		hashmap< const char*, const char*, hashstr, eq_str>::iterator iter = store.find(string);
		if (iter == store.end()) {
			/* duplicate string */
			cstr = strdup(string);
			/* insert into hashmap */
			store.insert(make_pair(cstr,cstr));
		} else
			/* return same string as before */
			cstr = iter->second;
		return;
	}

	std::ostream &operator<<(std::ostream &out, const ustring& str) {
		if (str.cstr) out << str.cstr;
		return out;
	}

	hashmap< const char*, const char*, hashstr, eq_str> ustring::store;
}
