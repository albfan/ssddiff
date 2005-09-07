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
#include <ext/hash_map>

/**** Utilities ****/
/* FIXME: move this into std namespace to get rid of GNU dependency? */
namespace __gnu_cxx {
/** \brief trivial "hash function" for pointers */
template<>
struct hash<void *> {
    /** \brief trivial "hash function" for pointers */
    size_t operator()(const void * __x) const {
	return reinterpret_cast<size_t>(__x); }
    };
}

/** \brief minimum macro */
#define MIN(a,b) (( (a<=b) ? a : b ))
/** \brief maximum macro */
#define MAX(a,b) (( (a>=b) ? a : b ))

/* vim:set noet sw=4 ts=8: */
#endif   /* ----- #ifndef UTIL_INC  ----- */
