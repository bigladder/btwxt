
#ifndef BTWXT_EXPORT_H
#define BTWXT_EXPORT_H

#ifdef BTWXT_STATIC_DEFINE
#  define BTWXT_EXPORT
#  define BTWXT_NO_EXPORT
#else
#  ifndef BTWXT_EXPORT
#    ifdef btwxt_EXPORTS
        /* We are building this library */
#      define BTWXT_EXPORT 
#    else
        /* We are using this library */
#      define BTWXT_EXPORT 
#    endif
#  endif

#  ifndef BTWXT_NO_EXPORT
#    define BTWXT_NO_EXPORT 
#  endif
#endif

#ifndef BTWXT_DEPRECATED
#  define BTWXT_DEPRECATED __attribute__ ((__deprecated__))
#endif

#ifndef BTWXT_DEPRECATED_EXPORT
#  define BTWXT_DEPRECATED_EXPORT BTWXT_EXPORT BTWXT_DEPRECATED
#endif

#ifndef BTWXT_DEPRECATED_NO_EXPORT
#  define BTWXT_DEPRECATED_NO_EXPORT BTWXT_NO_EXPORT BTWXT_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef BTWXT_NO_DEPRECATED
#    define BTWXT_NO_DEPRECATED
#  endif
#endif

#endif /* BTWXT_EXPORT_H */
