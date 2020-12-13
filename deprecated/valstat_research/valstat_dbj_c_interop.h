#ifndef	_DBJ_VALSTAT_C_INTEROP_
#define	_DBJ_VALSTAT_C_INTEROP_

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
namespace dbj {
	extern "C" {
#endif // __cplusplus

		/*
		The whole of valstat C interop is in macros bellow
		*/
#define valstat_interop_( T ) typedef struct interop_##T \
{ \
T * value ; \
const char * status  ; \
} interop_##T

#define valstat_interop( T ) valstat_interop_( T )

#define valstat_interop_decl_( T ) interop_##T 
#define valstat_interop_decl( T ) valstat_interop_decl_(T) 

		/**
		here we can define all the valstat for intrinsic types
		we might need
		*/

		/**
		this has created
			struct interop_void { void * value; const char * status; }
			*/
		valstat_interop(void);
		/**
		this has created
			struct interop_int { int * value; const char * status; }
			*/
		valstat_interop(int);
		valstat_interop(signed);
		valstat_interop(unsigned);
		valstat_interop(short);
		valstat_interop(long);

		valstat_interop(float);
		valstat_interop(double);

		valstat_interop(bool);

		valstat_interop(char);
#ifdef WIN32
		valstat_interop(wchar_t);
#endif

		/* stdint types */
		/* this has created
			struct interop_uint64_t  { uint64_t * value; const char * status; }
			*/
		valstat_interop(uint64_t);

#ifdef __cplusplus
	} // extern "C" 
} //	namespace dbj 
#endif // __cplusplus

/**
the status json for C interop tests
obviously works only if message is string literal 
thus users must not free it
a better but more complex mechanism would be to have a 
private registry/cache, hash table of strings
the will free itself on exit
*/
#undef _INTEROP_JSON_STRINGIZE_
#define _INTEROP_JSON_STRINGIZE_(x) #x

#undef  INTEROP_JSON_STRINGIZE
#define INTEROP_JSON_STRINGIZE(x) _INTEROP_JSON_STRINGIZE_(x)

#undef INTEROP_JSON
#define INTEROP_JSON(M) \
"{ \"file\": \"" __FILE__  "\", " \
 " \"line\": \""  INTEROP_JSON_STRINGIZE(__LINE__) "\", " \
 "\"message\": \"" M "\", \"timestamp\": \"" __TIMESTAMP__  "\" }" 

#endif	// _DBJ_VALSTAT_C_INTEROP_
