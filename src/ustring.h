/* ===========================================================================
 *        Filename:  ustring.h
 *     Description:  Unified String class
 * 
 *         Version:  $Rev$
 *         Changed:  $Date$
 *         Licence:  GPL (read COPYING file for details)
 * 
 *          Author:  Erich Schubert (eS), erich@debian.org
 *                   Institut für Informatik, LMU München
 * ======================================================================== */
#ifndef  USTRING_H
#define  USTRING_H

#include "config.h"
#include "util.h"

#include <iostream>
#include <map>
#include <cstring>

#include <libxml/tree.h> /* for xmlChar */

using namespace std;

namespace SSD {

/** \brief equality functor for char* strings */
struct eq_str {
	/** \brief equality for char* strings */
	bool operator()(const char* s1, const char* s2) const { return strcmp(s1, s2) == 0; }
};

/** \brief Class which stores each string only once and enables faster comparision */
/** by storing only one copy of each string, we can reduce string comparisions to
 *  pointer comparisions, which speeds up live a lot... */
class ustring {
	/** \brief string this represents */
	const char* cstr;

	/** \brief global store for "unified" strings */
	static hashmap< const char*, const char*, hashstr, eq_str> store;
public:
	/** \brief unify a char string */
	/** \param s char string to be unified */
	ustring(const char* s);
	/** \brief unify an xmlChar string */
	/** \param s xmlChar string to be unified */
	ustring(const xmlChar* s);

	/** \brief trivial compare operators using pointer value */
	/** \param other ustring to be compared with */
	bool operator==(const ustring other) const {
		return (cstr == other.cstr);
	}
	/** \brief trivial compare operators using pointer value */
	/** \param other ustring to be compared with */
	bool operator<=(const ustring other) const {
		return (cstr <= other.cstr);
	}
	/** \brief trivial compare operators using pointer value */
	/** \param other ustring to be compared with */
	bool operator<(const ustring other) const {
		return (cstr < other.cstr);
	}
	/** \brief trivial compare operators using pointer value */
	/** \param other ustring to be compared with */
	bool operator>=(const ustring other) const {
		return (cstr >= other.cstr);
	}
	/** \brief trivial compare operators using pointer value */
	/** \param other ustring to be compared with */
	bool operator>(const ustring other) const {
		return (cstr > other.cstr);
	}
	/** \brief trivial compare operators using pointer value */
	/** \param other ustring to be compared with */
	bool operator!=(const ustring other) const {
		return (cstr != other.cstr);
	}
	/** \brief test if this actually represents a nonempty string */
	bool empty() const { return (cstr == NULL); }
	/** \brief hash function is just using the pointer value */
	size_t hash() const { return reinterpret_cast<size_t>(cstr); }

	/** \brief append ustring to output stream for easier writing */
	/** \param out output stream to be appended to
	 *  \param str unified string to be appended */
	friend std::ostream &operator<<(std::ostream &out, const ustring &str);
};

/** \brief globally named "empty" string to compare with */
static ustring empty_ustring((char*)NULL);

/** \brief hash functor for ustrings in maps */
struct hash_ustring {
	/** \brief hash function for ustrings */
	/** \param u string to be hashed */
	size_t operator()(const SSD::ustring u) const {
		return u.hash();
	};
};
}

#endif   /* ----- #ifndef SSDDOC_H  ----- */
