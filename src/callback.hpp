#ifndef CALLBACK_HPP
#define CALLBACK_HPP

#include "log.hpp"

template<typename R=void, typename A0=int, typename A1=int, typename A2=int, typename A3=int>
struct Callback {
	virtual R operator()() const { ae_fatal("Abstract callback method reached"); }
	virtual R operator()(A0) const { ae_fatal("Abstract callback method reached"); }
	virtual R operator()(A0,A1,A2) const { ae_fatal("Abstract callback method reached"); }
	virtual R operator()(A0,A1,A2,A3) const { ae_fatal("Abstract callback method reached"); }
	template<typename Owner>
	class Arg1 : public Callback<R,A0> {
		mutable Owner* owner; mutable R (Owner::*fn)(A0);
	public: R operator ()(A0 a0) const { return (owner->*fn)(a0); }
		Arg1(Owner& owner, R (Owner::*fn)(A0)) : Callback<R,A0>(), owner(&owner), fn(fn) {} Arg1() {}
	};
	template<typename Owner>
	class Arg2 : public Callback<R,A0,A1> {
		mutable Owner* owner; mutable R (Owner::*fn)(A0,A1);
	public: R operator ()(A0 a0,A1 a1) const { return (owner->*fn)(a0,a1); }
		Arg2(Owner& owner, R (Owner::*fn)(A0,A1)) : Callback<R,A0,A1>(), owner(&owner), fn(fn) {} Arg2() {}
	};
	template<typename Owner>
	class Arg3 : public Callback<R,A0,A1,A2> {
		mutable Owner* owner; mutable R (Owner::*fn)(A0,A1,A2);
	public: R operator ()(A0 a0,A1 a1,A2 a2) const { return (owner->*fn)(a0,a1,a2); }
		Arg3(Owner& owner, R (Owner::*fn)(A0,A1,A2)) : Callback<R,A0,A1,A2>(), owner(&owner), fn(fn) {} Arg3() {}
	};
	template<typename Owner>
	class Arg4 : public Callback<R,A0,A1,A2,A3> {
		mutable Owner* owner; mutable R (Owner::*fn)(A0,A1,A2,A3);
	public: R operator ()(A0 a0,A1 a1,A2 a2,A3 a3) const { return (owner->*fn)(a0,a1,a2,a3); }
		Arg4(Owner& owner, R (Owner::*fn)(A0,A1,A2,A3)) : Callback<R,A0,A1,A2,A3>(), owner(&owner), fn(fn) {} Arg4() {}
	};


};

#endif
