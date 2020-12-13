#pragma once

#include "../valstat_dbj_c_interop.h"

#ifdef __cplusplus
namespace dbj {
	extern "C" {
#endif // __cplusplus

		/* type returned is: interop_double */
		valstat_interop_decl(double) log_valstat(double _X);
		valstat_interop_decl(double) log10_valstat(double _X);

		/*
		in C there is no overloading allowed, thus we can not use the
		log and log10 as function names
		*/

#ifdef __cplusplus
	} // extern "C"
} // namespace dbj 
#endif // __cplusplus

