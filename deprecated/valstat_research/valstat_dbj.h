#pragma once
/*
valstat 2 -- no pairs
*/

#include "valstat"
#include "../common.h"

#include <sstream> // this is bad :(

namespace dbj
{
	template<typename T>
	inline DBJ_STRING full_fat_str (T const & val_ )
	{
		std::ostringstream buf_;
		buf_ << val_;
		return buf_.str().c_str() ;
	}

	template< >
	inline DBJ_STRING full_fat_str<DBJ_STRING>
		(DBJ_STRING const & val )
	{
		return val;
	}

	/*
	dbj -- the imagined company
	this type alias is all they have and need to use std::valstat
	this makes them fully std::valstat compliant
	this also makes other people better understand their API's

	NOTE: DBJ_STRING is not the most performant json
		  string implementation. dbj, they usually use vector<char> or even
		  unique_ptr<char[]>
		  that issue is largely mitigated by not using strings
		  but handles to the strings registry as the status type
	*/
	template< typename T>
	using valstat = std::valstat<T, DBJ_STRING  >;

	// descriptive output
	// the  verbose valstat consuming
	// for testing purposes only
	// dbj usualy do not use <iostream> in a production code
	template<typename T>
	inline DBJ_STRING   to_string ( const dbj::valstat<T> & vt)
	{
		DBJ_STRING retval( "\nvalstat state: ");
		if (!vt.value && vt.status) retval +=  "ERROR";
		if (vt.value && !vt.status) retval += "OK";
		if (vt.value && vt.status) retval += "INFO";
		if (!vt.value && !vt.status) retval += "EMPTY";

		retval += " , content: \n{";
		if (vt.value)
		{
			retval += "\n { value: ";  retval += full_fat_str( *vt.value ) + " }";
		}
		else
			retval += "\n { value: empty }";
		retval += " ,";
		if (vt.status)
		{
			retval += "\n { status: "; retval += full_fat_str( *vt.status ) + " }";
		}
		else
			retval += "\n { status: empty }";
		return retval += "\n}\n";
	}

	/// <summary>
	/// fact of C++ computing is POSIX error codes are going to stay
	/// thus dbj have decalred the following valstat type too
	/// valstat_bj_status.h contains message
	/// creation from std::errc, part of <system_error>
	/// namespace dbj::posix {
	///		inline DBJ_STRING errc_to_message(std::errc posix_err_code);
	/// }
	/// used only when needed. 
	/// 
	/// Thus dbj pass the std::errc arround and get to its message
	/// only when needed
	/// 
	/// </summary>
	template<typename T>
	using posix_vs = std::valstat<T, std::errc >;

} // dbj

