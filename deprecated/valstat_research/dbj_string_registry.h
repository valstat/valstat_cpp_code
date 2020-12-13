#pragma once
// NOT IN USE CURRENTLY
#if 0
/*
table of strings
motivation: do not "carry arround" strings, but their 

TODO: resilient in the presence of threads
*/

#include <unordered_map>
#include <string_view>
#include <functional>
#include <iostream>
#include <cassert>

namespace dbj::registry {

	struct string_registry_handle;

	constexpr size_t string_registry_max_capacity = 0xFFFF;

	namespace inner {
		struct string_registry_type final {
			using key_type = std::size_t;
			using value_type = std::string;
			using storage_type = std::unordered_map< key_type, value_type >;
			storage_type storage_{};
			friend struct string_registry_handle;
		};
				inline string_registry_type string_registry_{};
	} // inner

	struct [[nodiscard]] string_registry_handle final 
	{
		std::size_t storage_id{};
		typename inner::string_registry_type::value_type const & 
			str() const noexcept
		{
			return inner::string_registry_.storage_.at(this->storage_id);
		}

		friend std::ostream& operator << (std::ostream & os, string_registry_handle const & handle_ )
		{
			return os << handle_.str().c_str();
		}
	};

	// TODO: template for all char types
	inline string_registry_handle add_string(std::string_view sv_)
	{
		using namespace std;
		static auto & storage = inner::string_registry_.storage_;

		assert(storage.size() < string_registry_max_capacity ) ;

		size_t new_key = hash<string_view>{}(sv_);
		auto rez = inner::string_registry_.storage_.insert( make_pair(new_key,string( sv_.data() ) ));
		return { new_key };
	}


} // dbj::registry

#endif