#pragma once

#include "../common.h"

namespace cpp03 {

	/// <summary>
	/// std free valstat 
	/// </summary>
	template<typename T, typename S>
	struct [[nodiscard]] valstat final
	{

		template< typename T_, typename OP >
		struct holder final
		{
			friend OP;
			using type = holder;
			using value_type = T_;

			friend T_ const& get(type const& holder_) noexcept
			{
				if (holder_)
					return holder_.v_;

				DBJ_PERROR;
				exit(-1);
			}

			constexpr T_ const* operator & () const noexcept {
				if (false == empty_) {
					return v_ ;
				}
				DBJ_PERROR;
				exit(-1);
			}

			constexpr operator bool() const noexcept {
				if (empty_) return false;
				return true;
			}

			~holder() { if (!empty_) { v_.~T_(); empty_ = true; } }
		private:
			bool empty_{ true };
			// T_ has to be default constructible, moveable, copyable
			T_ v_;
			void set(T_ new_v_) { v_ = new_v_; empty_ = false; }
		};

		using type = valstat;
		using value_type = T;
		using status_type = S;
		using value_proxy_type = holder<value_type, type>;
		using status_proxy_type = holder<status_type, type>;

		value_proxy_type	value;
		status_proxy_type	status;

		// 4 makers for 4 states
		static type ok(value_type value_arg_) noexcept
		{
			type retval_;
			retval_.value.set(value_arg_);
			// retval_.value(value_arg_);
			return retval_;
		}

		static type error(status_type status_arg_) noexcept
		{
			type retval_;
			retval_.status.set(status_arg_);
			return retval_;
		}

		static type info(value_type value_arg_, status_type status_arg_) noexcept
		{
			type retval_;
			retval_.value.set(value_arg_);
			retval_.status.set(status_arg_);
			return retval_;
		}

		static type empty() noexcept
		{
			type retval_;
			return retval_;
		}
	};

	/* usage ------------------------------------------------------------*/
	using int_vstat = cpp03::valstat<int, const char* >;

	int_vstat ref_signal(int& input_ref_)
	{
		using namespace std;

		if (input_ref_ < 42) {
			input_ref_ = SIG_ATOMIC_MAX;
			return int_vstat::ok(SIG_ATOMIC_MAX); // { value, empty }
		}
		return int_vstat::error("Input must NOT be bigger than magical constant");
	}

	TU_REGISTER([]
		{
			DBJ_PRINT("\n" DBJ_FG_CYAN_BOLD DBJ_FILE_LINE); DBJ_PRINT("\n" DBJ_RESET);

			auto driver = [](int arg)
			{
				using ::dbj::nanolib::ostrmng::prinf;
				auto [value, status] = ref_signal(arg);
				prinf(DBJ_FG_CYAN_BOLD);
				if (value && !status)
					prinf("\nOK state -- value: ", get(value), ", status: [empty]");

				if (value && status)
					prinf("\nINFO state -- value: ", get(value), ", status: ", get(status));

				if (!value && !status)
					prinf("\nEMPTY state --value: [empty], status : [empty] ");

				if (!value && status)
					prinf("\nERROR state -- value: [empty], status: ", get(status));
				prinf(DBJ_RESET);
			};

			driver(41);
			driver(43);

		});

}
