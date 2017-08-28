/* UnaryOperation.h -- Unary operations on Adept expressions

    Copyright (C) 2014-2016 European Centre for Medium-Range Weather Forecasts

    Robin Hogan <r.j.hogan@ecmwf.int>

    This file is part of the Adept library.
*/

#ifndef AdeptUnaryOperation_H
#define AdeptUnaryOperation_H

#include <adept/Expression.h>

#include <adept/ArrayWrapper.h>

namespace adept {

  namespace internal {

    // ---------------------------------------------------------------------
    // SECTION 3.1: Unary operations: define UnaryOperation type
    // ---------------------------------------------------------------------

    // Unary operations derive from this class, where Op is a policy
    // class defining how to implement the operation, and R is the
    // type of the argument of the operation
    template <typename Type, template<class> class Op, class R>
    struct UnaryOperation
      : public Expression<Type, UnaryOperation<Type, Op, R> >,
	protected Op<Type> {
      
      static const int  rank_      = R::rank;
      static const bool is_active_ = R::is_active && !is_same<Type,bool>::value;
      static const int  n_active_ = R::n_active;
      // FIX! Only store if active and if needed
      static const int  n_scratch_ = 1 + R::n_scratch;
      static const int  n_arrays_ = R::n_arrays;
      //      static const VectorOrientation vector_orientation_ = R::vector_orientation;
      // Will need to modify this for sqrt:
      static const bool is_vectorizable_ = false;

      using Op<Type>::operation;
      using Op<Type>::operation_string;
      using Op<Type>::derivative;
      
      //const R& arg;
      typename nested_expression<R>::type arg;

      UnaryOperation(const Expression<Type, R>& arg_)
	: arg(arg_.cast()) { }
      
      template <int Rank>
      bool get_dimensions_(ExpressionSize<Rank>& dim) const {
	return arg.get_dimensions(dim);
      }

//       Index get_dimension_with_len(Index len) const {
// 	return arg.get_dimension_with_len_(len);
//       }

      std::string expression_string_() const {
	std::string str;
	str = operation_string();
	//	str += "(" + static_cast<const R*>(&arg)->expression_string() + ")";
	str += "(" + arg.expression_string() + ")";
	return str;
      }

      bool is_aliased_(const Type* mem1, const Type* mem2) const {
	return arg.is_aliased(mem1, mem2);
      }
      bool all_arrays_contiguous_() const {
	return arg.all_arrays_contiguous_();
      }
      template <int n>
      int alignment_offset_() const { return arg.alignment_offset_<n>(); }

      /*
      template <int Rank>
      Type get(const ExpressionSize<Rank>& i) const {
	return operation(arg.get(i));
      }
      */

      template <int Rank>
      Type value_with_len_(Index i, Index len) const {
	return operation(arg.value_with_len(i, len));
      }
      /*
      template <int Rank>
      Type get_scalar() const {
	return operation(arg.get_scalar());
      }
      template <int Rank>
      Type get_scalar_with_len() const {
	return operation(arg.get_scalar_with_len());
      }
      */
      
      template <int MyArrayNum, int NArrays>
      void advance_location_(ExpressionSize<NArrays>& loc) const {
	arg.advance_location_<MyArrayNum>(loc);
      }

      template <int MyArrayNum, int NArrays>
      Type value_at_location_(const ExpressionSize<NArrays>& loc) const {
	return operation(arg.value_at_location_<MyArrayNum>(loc));
      }

      template <int MyArrayNum, int MyScratchNum, int NArrays, int NScratch>
      Type value_at_location_store_(const ExpressionSize<NArrays>& loc,
				    ScratchVector<NScratch>& scratch) const {
	return scratch[MyScratchNum] 
	  = operation(arg.value_at_location_store_<MyArrayNum,MyScratchNum+1>(loc, scratch));
      }

      template <int MyArrayNum, int MyScratchNum, int NArrays, int NScratch>
      Type value_stored_(const ExpressionSize<NArrays>& loc,
			 const ScratchVector<NScratch>& scratch) const {
	return scratch[MyScratchNum];
      }

      template <int MyArrayNum, int MyScratchNum, int NArrays, int NScratch>
      void calc_gradient_(Stack& stack, 
			  const ExpressionSize<NArrays>& loc,
			  const ScratchVector<NScratch>& scratch) const {
	arg.template calc_gradient_<MyArrayNum, MyScratchNum+1>(stack, loc, scratch,
		derivative(arg.value_stored_<MyArrayNum,MyScratchNum+1>(loc, scratch),
			   scratch[MyScratchNum]));
      }

      template <int MyArrayNum, int MyScratchNum, int NArrays, int NScratch,
		typename MyType>
      void calc_gradient_(Stack& stack, 
			  const ExpressionSize<NArrays>& loc,
			  const ScratchVector<NScratch>& scratch,
			  MyType multiplier) const {
	arg.template calc_gradient_<MyArrayNum, MyScratchNum+1>(stack, loc, scratch,
		multiplier*derivative(arg.value_stored_<MyArrayNum,MyScratchNum+1>(loc, scratch), 
				      scratch[MyScratchNum]));
      }

      template <int MyArrayNum, int Rank, int NArrays>
      void set_location_(const ExpressionSize<Rank>& i, 
			 ExpressionSize<NArrays>& index) const {
	arg.set_location_<MyArrayNum>(i, index);
      }

    }; // End UnaryOperation type
  
  } // End namespace internal

} // End namespace adept


// ---------------------------------------------------------------------
// SECTION 3.2: Unary operations: define specific operations
// ---------------------------------------------------------------------

// Overloads of mathematical functions only works if done in the
// global namespace
#define ADEPT_DEF_UNARY_FUNC(NAME, FUNC, RAWFUNC, STRING, DERIVATIVE)	\
  namespace adept{							\
    namespace internal {						\
      template <typename Type>						\
      struct NAME  {							\
	static const bool is_operator = false;				\
	const char* operation_string() const { return STRING; }		\
	Type operation(const Type& val) const {				\
	  return RAWFUNC(val);						\
	}								\
	Type derivative(const Type& val, const Type& result) const {	\
	  return DERIVATIVE;						\
	}								\
	Type fast_sqr(Type val) { return val*val; }			\
      };								\
    } /* End namespace internal */					\
  } /* End namespace adept */						\
  template <class Type, class R>					\
  inline								\
  adept::internal::UnaryOperation<Type, adept::internal::NAME, R>	\
  FUNC(const adept::Expression<Type, R>& r)	{			\
    return adept::internal::UnaryOperation<Type,			\
      adept::internal::NAME, R>(r.cast());				\
  }

// Functions y(x) whose derivative depends on the argument of the
// function, i.e. dy(x)/dx = f(x)
ADEPT_DEF_UNARY_FUNC(Log,   log,   log,   "log",   1.0/val)
ADEPT_DEF_UNARY_FUNC(Log10, log10, log10, "log10", 0.43429448190325182765/val)
ADEPT_DEF_UNARY_FUNC(Log2,  log2,  log2,  "log2",  1.44269504088896340737/val)
ADEPT_DEF_UNARY_FUNC(Sin,   sin,   sin,   "sin",   cos(val))
ADEPT_DEF_UNARY_FUNC(Cos,   cos,   cos,   "cos",   -sin(val))
ADEPT_DEF_UNARY_FUNC(Tan,   tan,   tan,   "tan",   1.0/fast_sqr(cos(val)))
ADEPT_DEF_UNARY_FUNC(Asin,  asin,  asin,  "asin",  1.0/sqrt(1.0-val*val))
ADEPT_DEF_UNARY_FUNC(Acos,  acos,  acos,  "acos",  -1.0/sqrt(1.0-val*val))
ADEPT_DEF_UNARY_FUNC(Atan,  atan,  atan,  "atan",  1.0/(1.0+val*val))
ADEPT_DEF_UNARY_FUNC(Sinh,  sinh,  sinh,  "sinh",  cosh(val))
ADEPT_DEF_UNARY_FUNC(Cosh,  cosh,  cosh,  "cosh",  sinh(val))
ADEPT_DEF_UNARY_FUNC(Abs,   abs,   std::abs, "abs", ((val>0.0)-(val<0.0)))
ADEPT_DEF_UNARY_FUNC(Fabs,  fabs,  std::abs, "fabs", ((val>0.0)-(val<0.0)))
ADEPT_DEF_UNARY_FUNC(Expm1, expm1, expm1, "expm1", exp(val))
ADEPT_DEF_UNARY_FUNC(Exp2,  exp2,  exp2,  "exp2",  0.6931471805599453094172321214581766*exp2(val))
ADEPT_DEF_UNARY_FUNC(Log1p, log1p, log1p, "log1p", 1.0/(1.0+val))
ADEPT_DEF_UNARY_FUNC(Asinh, asinh, asinh, "asinh", 1.0/sqrt(val*val+1.0))
ADEPT_DEF_UNARY_FUNC(Acosh, acosh, acosh, "acosh", 1.0/sqrt(val*val-1.0))
ADEPT_DEF_UNARY_FUNC(Atanh, atanh, atanh, "atanh", 1.0/(1.0-val*val))
ADEPT_DEF_UNARY_FUNC(Erf,   erf,   erf,   "erf",   1.12837916709551*exp(-val*val))
ADEPT_DEF_UNARY_FUNC(Erfc,  erfc,  erfc,  "erfc",  -1.12837916709551*exp(-val*val))

// Functions y(x) whose derivative depends on the result of the
// function, i.e. dy(x)/dx = f(y)
ADEPT_DEF_UNARY_FUNC(Exp,   exp,   exp,   "exp",   result)
ADEPT_DEF_UNARY_FUNC(Sqrt,  sqrt,  sqrt,  "sqrt",  0.5/result)
ADEPT_DEF_UNARY_FUNC(Cbrt,  cbrt,  cbrt,  "cbrt",  (1.0/3.0)/(result*result))
ADEPT_DEF_UNARY_FUNC(Tanh,  tanh,  tanh,  "tanh",  1.0 - result*result)

// Functions with zero derivative
ADEPT_DEF_UNARY_FUNC(Round, round, round, "round", 0.0)
ADEPT_DEF_UNARY_FUNC(Ceil,  ceil,  ceil,  "ceil",  0.0)
ADEPT_DEF_UNARY_FUNC(Floor, floor, floor, "floor", 0.0)
ADEPT_DEF_UNARY_FUNC(Trunc, trunc, trunc, "trunc", 0.0)
ADEPT_DEF_UNARY_FUNC(Rint,  rint,  rint,  "rint",  0.0)
ADEPT_DEF_UNARY_FUNC(Nearbyint,nearbyint,nearbyint,"nearbyint",0.0)

// Operators
ADEPT_DEF_UNARY_FUNC(UnaryPlus,  operator+, +, "+", 1.0)
ADEPT_DEF_UNARY_FUNC(UnaryMinus, operator-, -, "-", -1.0)
ADEPT_DEF_UNARY_FUNC(Not,        operator!, !, "!", 0.0)

//#undef ADEPT_DEF_UNARY_FUNC


// ---------------------------------------------------------------------
// SECTION 3.3: Unary operations: define noalias function
// ---------------------------------------------------------------------

namespace adept {
  namespace internal {
    // No-alias wrapper for enabling noalias()
    template <typename Type, class R>
    struct NoAlias
      : public Expression<Type, NoAlias<Type, R> > 
    {
      static const int  rank_      = R::rank;
      static const bool is_active_ = R::is_active;
      static const int  n_active_  = R::n_active;
      static const int  n_scratch_ = R::n_scratch;
      static const int  n_arrays_  = R::n_arrays;
      static const bool is_vectorizable_ = R::is_vectorizable;

      const R& arg;

      NoAlias(const Expression<Type, R>& arg_)
	: arg(arg_.cast()) { }
      
      template <int Rank>
	bool get_dimensions_(ExpressionSize<Rank>& dim) const {
	return arg.get_dimensions(dim);
      }

//       Index get_dimension_with_len(Index len) const {
// 	return arg.get_dimension_with_len_(len);
//       }

      std::string expression_string_() const {
	std::string str = "noalias(";
	str += static_cast<const R*>(&arg)->expression_string() + ")";
	return str;
      }

      bool is_aliased_(const Type* mem1, const Type* mem2) const {
	return false;
      }
      bool all_arrays_contiguous_() const {
	return arg.all_arrays_contiguous_(); 
      }
      template <int n>
      int alignment_offset_() const { return arg.alignment_offset_<n>(); }

      template <int Rank>
      Type value_with_len_(Index i, Index len) const {
	return operation(arg.value_with_len(i, len));
      }
      
      template <int MyArrayNum, int NArrays>
      void advance_location_(ExpressionSize<NArrays>& loc) const {
	arg.advance_location_<MyArrayNum>(loc);
      }

      template <int MyArrayNum, int NArrays>
      Type value_at_location_(const ExpressionSize<NArrays>& loc) const {
	return arg.value_at_location_<MyArrayNum>(loc);
      }
      template <int MyArrayNum, int NArrays>
      Packet<Type> packet_at_location_(const ExpressionSize<NArrays>& loc) const {
	return arg.packet_at_location_<MyArrayNum>(loc);
      }

      template <int MyArrayNum, int MyScratchNum, int NArrays, int NScratch>
      Type value_at_location_store_(const ExpressionSize<NArrays>& loc,
				    ScratchVector<NScratch>& scratch) const {
	return arg.value_at_location_store_<MyArrayNum,MyScratchNum>(loc, 
								     scratch);
      }

      template <int MyArrayNum, int MyScratchNum, int NArrays, int NScratch>
      Type value_stored_(const ExpressionSize<NArrays>& loc,
			 const ScratchVector<NScratch>& scratch) const {
	return scratch[MyScratchNum];
      }

      template <int MyArrayNum, int MyScratchNum, int NArrays, int NScratch>
      void calc_gradient_(Stack& stack, 
			  const ExpressionSize<NArrays>& loc,
			  const ScratchVector<NScratch>& scratch) const {
	arg.template calc_gradient_<MyArrayNum, MyScratchNum>(stack, loc, 
							      scratch);
      }

      template <int MyArrayNum, int MyScratchNum, int NArrays, int NScratch,
		typename MyType>
      void calc_gradient_(Stack& stack, 
			  const ExpressionSize<NArrays>& loc,
			  const ScratchVector<NScratch>& scratch,
			  MyType multiplier) const {
	arg.template calc_gradient_<MyArrayNum, MyScratchNum+1>(stack, loc, 
								scratch,
								multiplier);
      }

      template <int MyArrayNum, int Rank, int NArrays>
      void set_location_(const ExpressionSize<Rank>& i, 
			 ExpressionSize<NArrays>& index) const {
	arg.set_location_<MyArrayNum>(i, index);
      }

    }; // End struct NoAlias

  }

  template <typename Type, class R>
  inline
  adept::internal::NoAlias<Type, R>
  noalias(const Expression<Type, R>& r) {
    return adept::internal::NoAlias<Type, R>(r.cast());
  }

  template <typename Type>
  inline
  typename internal::enable_if<internal::is_not_expression<Type>::value, Type>::type
  noalias(const Type& r) {
    return r;
  }


  // ---------------------------------------------------------------------
  // SECTION 3.4: Unary operations: transpose function
  // ---------------------------------------------------------------------

  namespace internal {
    /*
    template <typename Type, class R>
    struct Transpose
      : public Expression<Type, Transpose<Type, R> > 
    {
      static const int  rank_      = 2;
      static const bool is_active_ = R::is_active;
      static const int  n_active_  = R::n_active;
      static const int  n_scratch_ = R::n_scratch;
      static const int  n_arrays_  = R::n_arrays;

      const R& arg;

      Transpose(const Expression<Type, R>& arg_)
	: arg(arg_.cast()) { }
      
      template <int Rank>
	bool get_dimensions_(ExpressionSize<Rank>& dim) const {
	return arg.get_dimensions(dim);
      }

      std::string expression_string_() const {
	std::string str = "transpose(";
	str += static_cast<const R*>(&arg)->expression_string() + ")";
	return str;
      }

      bool is_aliased_(const Type* mem1, const Type* mem2) const {
	return arg.is_aliased(mem1,mem2);
      }

      template <int Rank>
      Type value_with_len_(Index i, Index len) const {
	return operation(arg.value_with_len(i, len));
      }
      
      template <int MyArrayNum, int NArrays>
      void advance_location_(ExpressionSize<NArrays>& loc) const {
	arg.advance_location_<MyArrayNum>(loc);
      }

      template <int MyArrayNum, int NArrays>
      Type value_at_location_(const ExpressionSize<NArrays>& loc) const {
	return arg.value_at_location_<MyArrayNum>(loc);
      }

      template <int MyArrayNum, int MyScratchNum, int NArrays, int NScratch>
      Type value_at_location_store_(const ExpressionSize<NArrays>& loc,
				    ScratchVector<NScratch>& scratch) const {
	return arg.value_at_location_store_<MyArrayNum,MyScratchNum>(loc, 
								     scratch);
      }

      template <int MyArrayNum, int MyScratchNum, int NArrays, int NScratch>
      Type value_stored_(const ExpressionSize<NArrays>& loc,
			 const ScratchVector<NScratch>& scratch) const {
	return scratch[MyScratchNum];
      }

      template <int MyArrayNum, int MyScratchNum, int NArrays, int NScratch>
      void calc_gradient_(Stack& stack, 
			  const ExpressionSize<NArrays>& loc,
			  const ScratchVector<NScratch>& scratch) const {
	arg.template calc_gradient_<MyArrayNum, MyScratchNum>(stack, loc, 
							      scratch);
      }

      template <int MyArrayNum, int MyScratchNum, int NArrays, int NScratch,
		typename MyType>
      void calc_gradient_(Stack& stack, 
			  const ExpressionSize<NArrays>& loc,
			  const ScratchVector<NScratch>& scratch,
			  MyType multiplier) const {
	arg.template calc_gradient_<MyArrayNum, MyScratchNum+1>(stack, loc, 
								scratch,
								multiplier);
      }

      template <int MyArrayNum, int Rank, int NArrays>
      void set_location_(const ExpressionSize<Rank>& i, 
			 ExpressionSize<NArrays>& index) const {
	arg.set_location_<MyArrayNum>(i, index);
      }

    }; // End struct Transpose
    */

  }


  // ---------------------------------------------------------------------
  // SECTION 3.5: Unary operations: returning boolean expression
  // ---------------------------------------------------------------------
  namespace internal {

    // Unary operations returning bool derive from this class, where
    // Op is a policy class defining how to implement the operation,
    // and R is the type of the argument of the operation
    template <typename Type, template<class> class Op, class R>
    struct UnaryBoolOperation
      : public Expression<bool, UnaryBoolOperation<Type, Op, R> >,
	protected Op<Type> {
      
      static const int  rank_      = R::rank;
      static const bool is_active_ = false;
      static const int  n_active_ = 0;
      static const int  n_scratch_ = 0;
      static const int  n_arrays_ = R::n_arrays;
      
      using Op<Type>::operation;
      using Op<Type>::operation_string;
      
      const R& arg;

      UnaryBoolOperation(const Expression<Type, R>& arg_)
	: arg(arg_.cast()) { }
      
      template <int Rank>
      bool get_dimensions_(ExpressionSize<Rank>& dim) const {
	return arg.get_dimensions(dim);
      }

//       Index get_dimension_with_len(Index len) const {
// 	return arg.get_dimension_with_len_(len);
//       }

      std::string expression_string_() const {
	std::string str;
	str = operation_string();
	str += "(" + static_cast<const R*>(&arg)->expression_string() + ")";
	return str;
      }

      bool is_aliased_(const bool* mem1, const bool* mem2) const {
	return false;
      }
      bool all_arrays_contiguous_() const {
	return arg.all_arrays_contiguous_(); 
      }
      template <int n>
      int alignment_offset_() const { return arg.alignment_offset_<n>(); }

      template <int Rank>
      Type value_with_len_(Index i, Index len) const {
	return operation(arg.value_with_len(i, len));
      }
      
      template <int MyArrayNum, int NArrays>
      void advance_location_(ExpressionSize<NArrays>& loc) const {
	arg.advance_location_<MyArrayNum>(loc);
      }

      template <int MyArrayNum, int NArrays>
      Type value_at_location_(const ExpressionSize<NArrays>& loc) const {
	return operation(arg.value_at_location_<MyArrayNum>(loc));
      }

      template <int MyArrayNum, int MyScratchNum, int NArrays, int NScratch>
      Type value_at_location_store_(const ExpressionSize<NArrays>& loc,
				    ScratchVector<NScratch>& scratch) const {
	return scratch[MyScratchNum] 
	  = operation(arg.value_at_location_store_<MyArrayNum,MyScratchNum+1>(loc, scratch));
      }

      template <int MyArrayNum, int MyScratchNum, int NArrays, int NScratch>
      Type value_stored_(const ExpressionSize<NArrays>& loc,
			 const ScratchVector<NScratch>& scratch) const {
	return scratch[MyScratchNum];
      }

      template <int MyArrayNum, int MyScratchNum, int NArrays, int NScratch>
      void calc_gradient_(Stack& stack, 
			  const ExpressionSize<NArrays>& loc,
			  const ScratchVector<NScratch>& scratch) const { }

      template <int MyArrayNum, int MyScratchNum, int NArrays, int NScratch,
		typename MyType>
      void calc_gradient_(Stack& stack, 
			  const ExpressionSize<NArrays>& loc,
			  const ScratchVector<NScratch>& scratch,
			  MyType multiplier) const { }

      template <int MyArrayNum, int Rank, int NArrays>
      void set_location_(const ExpressionSize<Rank>& i, 
			 ExpressionSize<NArrays>& index) const {
	arg.set_location_<MyArrayNum>(i, index);
      }

    };
  
  } // End namespace internal
} // End namespace adept

// Overloads of mathematical functions only works if done in the
// global namespace
#define ADEPT_DEF_UNARY_BOOL_FUNC(NAME, FUNC, RAWFUNC)		\
  namespace adept {						\
    namespace internal {					\
      template <typename Type>					\
      struct NAME  {						\
	const char* operation_string() const { return #FUNC; }	\
	bool operation(const Type& val) const {			\
	  return RAWFUNC(val);					\
	}							\
      };							\
    } /* End namespace internal */				\
  } /* End namespace adept */					\
  template <class Type, class R>					\
  inline								\
  adept::internal::UnaryBoolOperation<Type, adept::internal::NAME, R>	\
  FUNC(const adept::Expression<Type, R>& r){				\
    return adept::internal::UnaryBoolOperation<Type,			\
      adept::internal::NAME, R>(r.cast());				\
  }

ADEPT_DEF_UNARY_BOOL_FUNC(IsNan,    isnan,    std::isnan)
ADEPT_DEF_UNARY_BOOL_FUNC(IsInf,    isinf,    std::isinf)
ADEPT_DEF_UNARY_BOOL_FUNC(IsFinite, isfinite, std::isfinite)

//#undef ADEPT_DEF_UNARY_BOOL_FUNC

  


#endif