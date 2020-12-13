#pragma once

#include "valstat_dbj_status.h"
#include "valstat_dbj.h"

#include <charconv>

namespace valstat_testing_space {

	using namespace std;
	/*
	putting valstat between users and std::from_chars
	*/
	template< typename T>
	inline dbj::valstat<T> convert(string_view sv) noexcept(true)
	{
		T rezult;
		if (auto [p, e] = from_chars(sv.data(), sv.data() + sv.size(), rezult);
			/* std::errc() is dubious hack from cppreference.com */
			e == std::errc()
			)
		{
			// valstat info state
			return { {rezult}, { dbj::make_status(__FILE__, __LINE__, __TIMESTAMP__)  } };
		}
		else {
			// valstat error state
			return { {}, { dbj::make_status(__FILE__, __LINE__, __TIMESTAMP__)  } };
		}
	}
	// Test Unit aka "Unit Test" ;)
	TU_REGISTER([] {
		DBJ_PRINT(DBJ_FG_CYAN_BOLD DBJ_FILE_LINE); DBJ_PRINT(" " DBJ_RESET);

		using namespace std::literals;
		using namespace testing_space;
		driver(
			[&] { return convert<int>("42"sv); }, "convert<int>(\"42\"sv)"
		);
		// return result 4.2 as int 4
		driver(
			[&] { return convert<int>("4.2"sv); }, "convert<int>(\"4.2\"sv)"
		);
		}
	);

	/*
	-----------------------------------------------------------------------
	PARADIGM SHIFTING -- opearator returning valstat makes for richer
	return producing and return consuming logic
	*/
	template< size_t N>
	struct arry final
	{
		array<char, N> buff_{ 0 };

		// valstat return does not require
		// exception thinking
		// there is always a return
		dbj::valstat<char> operator [] (size_t idx_) const noexcept
		{
			if (idx_ >= buff_.size())
				return { {}, { dbj::make_status(__FILE__, __LINE__, __TIMESTAMP__,"Index out of bounds") } };

			return { { buff_[idx_] } , {} };
		}
	}; // arry

	TU_REGISTER([] {
		DBJ_PRINT(DBJ_FG_CYAN_BOLD DBJ_FILE_LINE); DBJ_PRINT(" " DBJ_RESET);

		static arry<0xFF> xarr{ {"0124356ABCDEFH"} };

		using namespace testing_space;
		driver(
			[&] {  return xarr[7]; },
			"arry<0xFF> xarr{ {\"0124356ABCDEFH\"} }; xarr[7]"
		);
		});

	/*
	(c) dbj@dbj.org
	literal to std::array
	but no strings until C++ 2.x
	*/
	template< char ... Chs >
	inline constexpr decltype(auto) operator"" _charay()
	{
		// append '\0'
		return  std::array{ Chs..., char(0) };
	}

	template< char ... Chs >
	inline constexpr decltype(auto) operator"" _conv()
	{
		// append '\0'
		return arry<1 + sizeof...(Chs)>{ Chs..., char(0) };
	}


#if defined(__clang__ ) || defined(__llvm__) || defined(__GNUC__)
	// https://wandbox.org/permlink/ubNTUYDrs2NEaDFz
	// yes we can have valstat returned from UDL 
	template< char ... Chs >
	inline constexpr decltype(auto) operator"" _to_valstat()
	{
		using rtype = arry<1 + sizeof...(Chs)>;
		using vt = dbj::valstat<rtype>;
		// append '\0'
		return vt{ { rtype{ Chs..., char(0) } } , {} };
	}

	template< char ... Chs >
	inline constexpr decltype(auto) operator"" _charray_valstat()
	{
		using val_type = std::array<char, 1 + sizeof...(Chs)>;
		using vstt = dbj::valstat<val_type>;

		if (sizeof...(Chs) > 42)
			// ERR state
			return vstt{ {}, "Do not go over 42!" };
		// OK state
		return vstt{ val_type{ Chs..., char(0) } , {} };
	}

	TU_REGISTER([] {
		DBJ_PRINT(DBJ_FG_CYAN_BOLD DBJ_FILE_LINE); DBJ_PRINT(" " DBJ_RESET);

		auto [val, stat] = 123_charray_valstat;

		if (val) {
			cout << *val;
		}

		if (stat) {
			cout << *stat;
		}

		});

	TU_REGISTER([] {
		DBJ_PRINT(DBJ_FG_CYAN_BOLD DBJ_FILE_LINE); DBJ_PRINT(" " DBJ_RESET);

		auto [val, stat] = 0xFFF_to_valstat;
		using namespace dbj; /* pacify ADL */
		if (val) {
			cout << endl << (*val)[1];
		}
		});
#endif // __clang__

	TU_REGISTER([] {
		DBJ_PRINT(DBJ_FG_CYAN_BOLD DBJ_FILE_LINE); DBJ_PRINT(" " DBJ_RESET);

		using namespace dbj;
		constexpr auto ar = 123_conv;
		// paradigm shift, no exception logic, local handling
		cout << endl << ar[5];
		cout << endl << ar[0];

		});
	/*
	handling no copy/no move types
	 */
	struct adamant final {
		inline static const char* genus = "tenacious";

		adamant() = default;

		adamant(const adamant&) = delete;
		adamant& operator = (const adamant&) = delete;
		adamant(adamant&&) = delete;
		adamant& operator = (adamant&&) = delete;

		// type::vt
		// is the encapsulated valstat for the type
#if 0
		using vt = dbj::valstat< adamant >;
#else
		using vt = dbj::valstat< reference_wrapper<adamant> >; // clang
#endif

		friend ostream& operator << (ostream& os, const adamant& vt)
		{
			return os << "adamant::genus = " << vt.genus;
		}
	};

	TU_REGISTER([] {
		DBJ_PRINT(DBJ_FG_CYAN_BOLD DBJ_FILE_LINE); DBJ_PRINT(" " DBJ_RESET);

		adamant steadfast{};

		auto info = [&]() -> adamant::vt { return  { steadfast, { "info message" } }; };

		auto error = [&]() -> adamant::vt { return  { {}, { "error message" } }; };

		auto ok = [&]() -> adamant::vt { return  { steadfast , {} }; };

		auto empty = [&]() -> adamant::vt { return  { {}, {} }; };

		auto consumer = [](auto producer) {
			using namespace dbj;
			// auto [val, stat] = producer();
			// sampling through the verbose stream output op.
			cout << producer();
		};

		consumer(info);
		consumer(error);
		consumer(ok);
		consumer(empty);

		});

	dbj::valstat<reference_wrapper<int> > ref_signal(int& input_ref_)
	{
		input_ref_ = SIG_ATOMIC_MAX;
		return { {input_ref_} , {} };
	}

	TU_REGISTER([] {
		DBJ_PRINT(DBJ_FG_CYAN_BOLD DBJ_FILE_LINE); DBJ_PRINT(" " DBJ_RESET);

		int arg = 0;
		auto [val, stat] = ref_signal(arg);
		DBJ_ASSERT(*val == SIG_ATOMIC_MAX);
		});

	namespace {
		// #define X_REF_VALSTAT 
		struct X
		{
			constexpr static int special = 42;
			int val{ -special }; //example: special value -> means default constructed
			X(X&&) = delete;
			X(X const&) = delete;
			X(int i) : val(i% special) {}
			X() : val{ 0 } {} // default ctor
			void method() { val += special; }
#ifdef X_REF_VALSTAT
			using vstat = dbj::valstat< reference_wrapper<X> >;
#else
			using vstat = dbj::valstat<X>;
#endif
			static X::vstat make(int v_)
			{
#ifdef X_REF_VALSTAT
				static X x_(v_); // bug
				return { x_ , {} };
#else
				return { v_ , {} };
#endif
			}
		};

		struct Y { constexpr auto special() { return 42; } };

		TU_REGISTER([]
			{
				DBJ_PRINT(DBJ_FG_CYAN_BOLD DBJ_FILE_LINE); DBJ_PRINT(" " DBJ_RESET);

				{
					optional<Y> opty = Y{};
					static_assert(42 == opty->special());

					dbj::valstat<Y> valty;
					static_assert(42 == valty.value->special());

					auto tricky = []() -> dbj::valstat<Y> {
						// agregate and return { Y{} ,  empty } valstat
						return { Y{}, {} };
					};

					auto [v, s] = tricky();

					static_assert(42 == v->special()); // returns 42
				}

				// usage stays the same
				// regalrdes of value being reference or not
				auto [value, status] = X::make(4);
				X& xref = *value;
				DBJ_ASSERT(xref.val == 4);
				xref.method();
			});
	}

	namespace rzej_challenge
	{
		template <typename F>
		dbj::valstat<std::invoke_result<F>> call(F fun)
		{
			if (true)
				// DBJ: f() can throw exception?
				// DBJ: f() can have arguments...
				return { {fun()}, {} };
			else
				return { {}, {"error"} };
		};

		template <typename T> // may be a reference or a value
		void use()
		{
			auto f = []() { return T{};  };
			// val may be a reference wrapper or not
			auto [val, sat] = call(f);
			if (val)
			{
				(*val).method();
				// val->get().method(); // which one should I call?
			}
		}
	}

} // dbj




