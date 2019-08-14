#include <iostream>
#include <string>
#include "assert.h"

#include "boolector_factory.h"
#include "smt.h"
// after a full installation
// #include "smt-switch/boolector_factory.h"
// #include "smt-switch/smt.h"

using namespace smt;
using namespace std;

int main()
{
  unsigned int NUM_TERMS = 20;

  SmtSolver s = BoolectorSolverFactory::create();
  s->set_opt("produce-models", true);
  Sort bvsort8 = s->make_sort(BV, 8);

  UnorderedTermSet uts;
  UnorderedTermMap utm;
  TermVec v;
  v.reserve(NUM_TERMS);
  Term x;
  Term y;
  for (size_t i = 0; i < NUM_TERMS; ++i)
  {
    x = s->declare_const("x" + to_string(i), bvsort8);
    y = s->declare_const("y" + to_string(i), bvsort8);
    v.push_back(x);
    uts.insert(x);
    utm[x] = y;
  }

  Term trailing = v[0];
  for (size_t i = 1; i < NUM_TERMS; ++i)
  {
    s->assert_formula(s->apply(
        Equal, v[i], s->apply(BVAdd, trailing, s->make_value(1, bvsort8))));
    trailing = v[i];
  }

  Term zero = s->make_value(0, bvsort8);
  cout << zero->to_string() << endl;

  assert(zero->is_value());
  assert(!v[0]->is_value());
  assert(v[0]->is_symbolic_const());

  Term v0_eq_0 = s->apply(Equal, v[0], zero);
  s->assert_formula(v0_eq_0);

  cout << "Children of term:" << endl;
  // Could use iterators directly:
  //   for (TermIter it = v0_eq_0->begin(); it != v0_eq_0->end(); ++it)
  // Or use a range-based loop
  for (auto c : v0_eq_0)
  {
    cout << "got: " << c << endl;
  }

  // just assign all ys to x counterparts
  for (auto it = uts.begin(); it != uts.end(); ++it)
  {
    x = *it;
    y = utm.at(*it);
    std::cout << "assert: " << x << " = " << y << std::endl;
    s->assert_formula(s->apply(Equal, x, y));
  }

  bool res = s->check_sat().is_sat();
  assert(res);

  assert(v[0]->is_symbolic_const());

  // can print variable names, but otherwise boolector doesn't maintain strings
  // for expressions
  cout << "Assignments:" << std::endl;
  for (size_t i = 0; i < NUM_TERMS; ++i)
  {
    cout << "\t " << v[i]->to_string() << " = "
         << s->get_value(v[i])->to_int() << endl;
    cout << "\t " << utm.at(v[i])->to_string() << " = "
         << s->get_value(utm.at(v[i]))->to_int() << endl;
  }
  return 0;
}
