#pragma once

#include "../common.h"
#include "../valstat_research/valstat_dbj_status.h"
/// #include <valstat>

/**
Apply the valstat to some beginers/intermediate C++ code
Basically minimize asserts, maximize the resilience ...

NOTE: very often beginners code looks like C
*/

namespace valstat_testing_space {

	using namespace std;
	using dbj::print ;

	/// <summary>
	/// We declare status to be std::string
	/// this declaration is all we nead to reap the benefits of valstat
	/// </summary>
	template<typename T>
	using polygon_vstat = std::valstat<T, std::string >;

/// <summary>
/// Polygon is a list of points
/// this 2D Point as a POD
/// </summary>
	struct Point
	{
		int TAG = 0;
		int x = 0;
		int y = 0;

		using pointer = std::shared_ptr<Point>;

		pointer next = nullptr;
	};

	inline Point::pointer  root_point() noexcept {
		// we terminate on bad alloc
		return std::make_shared<Point>(); 
	};

	inline polygon_vstat<double> polygon_area(Point::pointer pl_) noexcept
	{
		if (!pl_)
			return { {} , "nullptr argument?"  }; // ERROR state return

		double area = 0.0;
		Point::pointer p_ = pl_;
		while (p_->next)
		{
			area += (p_->x * p_->next->y - p_->next->x * p_->y);
			p_ = p_->next;
		}
		area = area / 2;
		area = (area > 0 ? area : -1 * area);
		return { area, {} }; // return OK state
	}

	inline polygon_vstat<int> random_number(int min_num, int max_num) noexcept
	{
		if (min_num > max_num)
			return { {} ,  " min arg is biger than max arg?"  };

		return { (rand() % (max_num - min_num)) + min_num , {} };
	}

	// no can do -- void as the value type
	inline void point_print_all(Point::pointer pl_) noexcept
	{
		while (pl_ ) {
			print("\n\nPoint { [%d] %d, %d, %p }", pl_->TAG, pl_->x, pl_->y, pl_->next.get());
			pl_ = pl_->next;
		}
	}

	inline void point_rmv(Point::pointer pl_) noexcept
	{
		while (pl_ ) {
			print("\n\nRMV-ing Point { (%d, %d, %d), next: %p }", pl_->TAG, pl_->x, pl_->y, pl_->next.get());
			auto tmp = pl_ ;
			pl_ = pl_->next;
			tmp = nullptr;
		}
	}

	// no reason for valstat here
	// make_shared_will abort if no memory
	inline Point::pointer point_new(int tag, int x, int y) noexcept
	{
		// we terminate on heap axhaustion
		Point::pointer newp = std::make_shared<Point>();
		// { tag, x, y, nullptr }
		newp->TAG = tag;
		newp->x = x;
		newp->y = y;
		return newp;
	}

	inline  polygon_vstat<Point::pointer> point_add(Point::pointer last, int tag_, int x, int y) noexcept
	{
	if (!last)	return { {} , "Invalid argument" };
		// point will be always here
		auto point = point_new(tag_, x, y);
			last->next = point;
	return { last->next, {} }; // OK state
	}

	/*
	in the first quadrant, fenced by 1000 x 1000 square, one corner at origin
	*/
	inline polygon_vstat<Point::pointer> make_random_polygon(unsigned side_num, Point::pointer first_)
	{
		if (!first_)
			return { {}, "null argument ?" };

		Point::pointer last_ = first_;
		for (unsigned int tag = 0; tag < side_num; tag++)
		{
			/*
			sequence of calls returning valstat's each
			using new if() syntax clarifies the code

			Is this simple? No, it is not.
			Is this resilient? Yes, it is.
			*/
			if (auto [rnd_1, state_1] = random_number(0, 1000); state_1) return { {}, state_1 };
			else
				if (auto [rnd_2, state_2] = random_number(0, 1000); state_2) return { {}, state_2 };
				else
					if (auto [point, state_3] = point_add(last_, tag + 1, *rnd_1, *rnd_2); state_3) return { {},  state_3 };
					else
						last_ = *point;
		}
		return { first_, {} }; // OK
	}

	inline polygon_vstat<Point::pointer> make_unit_square(Point::pointer first_, unsigned side_length = 1)
	{
		if (!first_) return { {},  "nullptr argument?" };
		Point::pointer last_ = first_;

		if (auto [p, e] = point_add(last_, 0, 0, 0); e) return { {}, e };// lower left
		else last_ = *p;

		if (auto [p, e] = point_add(last_, 1, 0, 1); e) return { {}, e };// lower right
		else last_ = *p;

		if (auto [p, e] = point_add(last_, 2, 1, 1); e) return { {}, e };// upper right
		else last_ = *p;

		if (auto [p, e] = point_add(last_, 3, 1, 0); e) return { {}, e };// upper left
		else last_ = *p;

		return { first_ , {} };
	}

	TU_REGISTER(
		[] 
		{
			DBJ_PRINT(DBJ_FG_CYAN_BOLD DBJ_FILE_LINE); DBJ_PRINT(" " DBJ_RESET);

			// __int32 and __int64 are cl.exe intrinsics
			srand((__int32)time(nullptr));
			auto root_ = root_point();

			if (auto [point_, e_] = make_unit_square( root_); e_) print("Error %s", (*e_).c_str());
				else {
					// point_print_all(*point_);
					if (auto [area, e_] = polygon_area((*point_)->next); e_) print("Error %s", (*e_).c_str());
					else
						print("\n\nThe area of the unit square is %0.2lf  'units'\n", *area);
				};

			// another example -- we simply forget about the previous root_
			root_ = root_point(); 

			// valstat consumption
			if ( auto [ p, e ] = make_random_polygon(42, root_); e)  print("Error %s", (*e).c_str());
			else {
				// point_print_all(*p);
				if (auto [area, e_] = polygon_area((*p)->next); e_) print("Error %s", (*e_).c_str());
				else
					print("\n\nThe area of the unit square is %0.2lf 'units' \n", *area);
			};

			// again, let's forget about the root pointer
		});
	} // namespace valstat_testing_space 
