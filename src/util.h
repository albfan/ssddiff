/* ===========================================================================
 *        Filename:  util.h
 *     Description:  Header with utility classes and definitions
 * 
 *         Version:  $Rev$
 *         Changed:  $Date$
 *         Licence:  GPL (read COPYING file for details)
 * 
 *          Author:  Erich Schubert (eS), erich@debian.org
 *                   Institut für Informatik, LMU München
 * ======================================================================== */
#ifndef  UTIL_INC
#define  UTIL_INC

#include <string.h>
#include <tr1/unordered_map>

/** \brief Hashmap implementation to use */
#define hashmap std::tr1::unordered_map
/** \brief Hash implementation to use */
#define hashfun std::tr1::hash

/** \brief minimum macro */
#define MIN(a,b) (( (a<=b) ? a : b ))
/** \brief maximum macro */
#define MAX(a,b) (( (a>=b) ? a : b ))

/** \brief hash function for char* 'strings' */
struct hashstr {
	std::size_t operator()(const char* s) const {
		/* I'm not sure how portable this references is.
		 * It's accessing an internal part of the TR1 implementation */
		return std::tr1::_Fnv_hash<>::hash(s, strlen(s));
	}
};

/* vim:set noet sw=4 ts=8: */
#endif   /* ----- #ifndef UTIL_INC  ----- */
