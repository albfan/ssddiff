/* ===========================================================================
 *        Filename:  config.h
 *     Description:  Configuration Options
 * 
 *         Version:  $Rev$
 *         Changed:  $Date$
 *         Licence:  GPL (read COPYING file for details)
 * 
 *          Author:  Erich Schubert (eS), erich@debian.org
 *                   Institut fÃ¼r Informatik, LMU MÃ¼nchen
 * ========================================================================= */
/* keep these enabled for now, algorithm might not work otherwise */
#define NEED_INDEX
#define NEED_PROCESSED_SET
/* debug options */
//#define VERBOSE_COSTS
//#define VERBOSE_COSTS_2
//#define VERBOSE_COSTS_3
//#define VERBOSE_COSTS_4

/* Sequence Counting is required for good tracing of the progress, but it
 * adds extra bytes to each astarstate (i.e. open end),
 * thus increasing memory usage */
#undef VERBOSE_SEQCOUNT

/* Tracing will provide an option for extra debug output */
#define TRACING_ENABLED

/* dependencies: */
#ifdef TRACING_ENABLED
# define VERBOSE_SEQCOUNT
#endif
