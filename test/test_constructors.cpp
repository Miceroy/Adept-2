/* test_constructors.cpp - Test Adept's selection of constructors in a range of scenarios

    Copyright (C) 2017 European Centre for Medium-Range Weather Forecasts

    Author: Robin Hogan <r.j.hogan@ecmwf.int>

  Copying and distribution of this file, with or without modification,
  are permitted in any medium without royalty provided the copyright
  notice and this notice are preserved.  This file is offered as-is,
  without any warranty.
*/

#include <iostream>

#define ADEPT_BOUNDS_CHECKING 1
#define ADEPT_VERBOSE_FUNCTIONS
#define ADEPT_NO_ALIAS_CHECKING

#include <adept_arrays.h>

using namespace adept;

Vector square(const Vector& v) {
  std::cout << "  inside function\n";
  return v*v;
}

void square_in_place(Vector& v) {
  std::cout << "  inside function\n";
  v *= v;
}

Vector square_copy(Vector v) {
  std::cout << "  inside function\n";
  v *= -1.0;
  return v*v;
}

#define EVAL(MSG,X,COMMAND) std::cout << "--------------------------------------------------------------------\n" \
  << MSG << "\n" \
  << #X << " = " << X << "\n" \
  << #COMMAND << "\n"; \
  COMMAND; \
  std::cout << #X << " = " << X << "\n"

 #define EVAL2(MSG,X,COMMAND,Y) std::cout << "--------------------------------------------------------------------\n" \
  << MSG << "\n" \
  << #X << " = " << X << "\n" \
  << #COMMAND << "\n"; \
  COMMAND;					\
  std::cout << #X << " = " << X << "\n" \
            << #Y << " = " << Y << "\n"

#define EVAL_FAIL(MSG,X,COMMAND) std::cout << "--------------------------------------------------------------------\n" \
  << MSG << "\n" \
  << #COMMAND << "\n" \
  << "DOES NOT COMPILE (INCORRECT BEHAVIOUR)\n"

#define EVAL2_FAIL(MSG,X,COMMAND,Y) std::cout << "--------------------------------------------------------------------\n" \
  << MSG << "\n" \
  << #COMMAND << "\n" \
  << "DOES NOT COMPILE (INCORRECT BEHAVIOUR)\n"

#define VERDICT98(MSG) std::cout << "Verdict for C++98: " << MSG << "\n"
#define VERDICT11(MSG) std::cout << "Verdict for C++11: " << MSG << "\n"

#define HEADING(MSG) std::cout << "####################################################################\n" \
  << MSG << "\n"


int
main() {

  Vector v(2), w(2), v_const_data(2);
  v = 2.0;
  v_const_data = 3.0;
  const Vector v_const = v_const_data;

  {
  HEADING("COPY CONSTRUCTORS");
  EVAL2("Passing Vector as argument to Vector copy constructor",
	v, const Vector v2(v), v2);
  VERDICT98("correct");
  VERDICT11("should perform deep copy");

  EVAL2("Passing Vector as argument to const Vector copy constructor",
	v, const Vector v_const(v), v_const);
  VERDICT98("correct");
  VERDICT11("should perform deep copy");

  EVAL2("Passing const Vector as argument to const Vector copy constructor",
	v_const, const Vector v_const2(v_const), v_const2);
  VERDICT98("correct");
  VERDICT11("should perform deep copy");

  EVAL2("Passing const Vector as argument to Vector copy constructor",
	v_const, Vector v3(v_const), v3);
  VERDICT98("should not compile");
  VERDICT11("should perform deep copy");
  }

  HEADING("ASSIGNMENT OPERATOR");
  EVAL2("Passing Vector to assignment operator",
	v, w = v, w);
  EVAL2("Passing const Vector to assignment operator",
	v_const, w = v_const, w);
  EVAL2("Passing Vector rvalue to assignment operator",
	v, w = v(stride(1,0,-1)), w);
  EVAL2("Passing const-Vector rvalue to assignment operator",
	v_const, w = v_const(stride(1,0,-1)), w);
  EVAL2("Passing Expression to assignment operator",
	v, w = v+v, w);

  HEADING("PASSING Vector TO FUNCTIONS");
  EVAL2("Passing Vector as argument to function taking const Vector&",
       v, w = square(v), w);
  VERDICT98("too many copies");
  VERDICT11("could replace last copy with a move");
  EVAL("Passing Vector as argument to function taking Vector&",
       v, square_in_place(v));
  VERDICT98("correct");

  v = 2.0;
  EVAL2("Passing Vector as argument to function taking Vector",
       v, w = square_copy(v), w);
  VERDICT98("too many copies, unexpected change of argument");
  VERDICT11("should do deep copy on input, replace last copy with a move");

  /*

    // Behaves same as passing non-const Vector, which is correct

  // Passing const Vector
  EVAL2("Passing const Vector as argument to function taking const Vector&",
       v_const, w = square(v_const), w);
  // The following should not compile:
  //  EVAL("Passing const Vector as argument to function taking Vector&",
  //       v_const, square_in_place(v_const));
  EVAL2("Passing const Vector as argument to function taking Vector",
       v_const, w = square_copy(v_const), w);

  */

  HEADING("PASSING Vector RVALUE TO FUNCTIONS");
  EVAL2("Passing Vector rvalue as argument to function taking const Vector&",
	v, w = square(v(stride(1,0,-1))), w);
  VERDICT98("correct");
  EVAL_FAIL("Passing Vector rvalue as argument to function taking Vector&",
       v, square_in_place(v(stride(1,0,-1))));
  VERDICT98("Vector subset functions could return references?");

  v = 2.0;
  EVAL2("Passing Vector rvalue as argument to function taking Vector",
	     v, w = square_copy(v(stride(1,0,-1))), w);
  VERDICT98("Vector subset functions could return references?");
  VERDICT11("Should use move function");

  HEADING("PASSING const Vector RVALUES TO FUNCTIONS");
  EVAL2("Passing const-Vector rvalue as argument to function taking const Vector&",
	v_const, w = square(v_const(stride(1,0,-1))), w);
  VERDICT98("correct");
  // This should not compile
  //  EVAL("Passing const-Vector rvalue as argument to function taking Vector&",
  //       v_const, square_in_place(v_const(stride(1,0,-1))));
  //  VERDICT98("Vector subset functions could return references?");
  EVAL2("Passing const-Vector rvalue as argument to function taking Vector",
	     v_const, w = square_copy(v_const(stride(1,0,-1))), w);
  VERDICT98("correct");
  //  VERDICT11("Should use move function");

  HEADING("PASSING Expression TO FUNCTIONS");
  EVAL2("Passing Expression as argument to function taking const Vector&",
       v, w = square(v+v), w);
  VERDICT98("Unclear why copy-assignment + constructor needed");
  // This should not compile:
  //  EVAL("Passing Expression as argument to function taking Vector&",
  //       v, square_in_place(v+v));
  v = 2.0;
  EVAL2("Passing Expression as argument to function taking Vector",
       v, w = square_copy(v+v), w);
  VERDICT98("Unclear why copy-assignment + constructor needed");

  return 0;
}
