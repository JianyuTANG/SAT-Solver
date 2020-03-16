//
// DPLL algorithm.
//

#ifndef DPLL_DPLL_H
#define DPLL_DPLL_H

#include "common.h"
#include <stack>
#include <set>
#include <random>
#include <algorithm>

#define DECIDE 0
#define BACKTRACK 1
#define PROPAGATE 2
#define SAT 0
#define CONFLICT 1
#define OTHERS 2


struct choice
{
	literal variable;
	int category;
	choice(literal v, int cat) :variable(v), category(cat) {
	}
};


class DPLL {
public:
    /**
     * Constructor.
     *
     * @param phi the formula to be checked
     * @note Please DON'T CHANGE this signature because the grading script will directly call this function!
     */
    DPLL(const formula &phi) : phi(phi), cur_time(0),
		remaining_var(phi.num_variable), 
		roots(phi.num_variable + 1, std::set<int>()),
		time_stamp(phi.num_variable + 1, 0),
		mp(phi.num_variable + 1, 0),
		visited(phi.num_variable + 1, 0),
		related_clauses(phi.num_variable + 1, std::vector<int>())
	{
		int n = phi.clauses.size();
		for (int i = 0; i < n; i++)
			cur_space.insert(i);
		/*for (auto iter=this->phi.clauses.begin();iter!=this->phi.clauses.end();iter++)
		{
			std::sort((*iter).begin(), (*iter).end(), [](int x, int y) {
				return VAR(x) < VAR(y);
				});
		}
		std::sort(phi.clauses.begin(), phi.clauses.end(), [](const auto& x, const auto& y) {
			return VAR(x[0]) < VAR(y[0]);
			});*/
	}

    /**
     * Check if the formula is satisfiable.
     *
     * @return true if satisfiable, and false if unsatisfiable
     * @note Please DON'T CHANGE this signature because the grading script will directly call this function!
     */
    bool check_sat();

    /**
     * Get a satisfying model (interpretation) of the formula, the model must be *complete*, that is,
     * it must assign every variable a truth value.
     * This function will be called if and only if `check_sat()` returns true.
     *
     * @return an arbitrary (if there exist many) satisfying model
     * @note Please DON'T CHANGE this signature because the grading script will directly call this function!
     */
    model get_model();

	bool check_sat_noncdcl();
	literal propagate();
	literal propagate_noncdcl();
	bool try_backtrack();
	bool try_backjump();
	void make_decision(std::default_random_engine& random, std::bernoulli_distribution& d);
	void make_decision_noncdcl(std::default_random_engine& random, std::bernoulli_distribution& d);
	void traverse_clauses();

private:
    formula phi;
	int remaining_var;
	std::stack<choice> st;
	std::stack<int> decision_st;
	std::vector<int> mp, visited;
	std::vector<std::set<int>> roots;
	unsigned long long cur_time;
	std::vector<unsigned long long> time_stamp;
	std::vector<std::vector<int>> related_clauses;
	std::set<int> cur_space;
};


#endif //DPLL_DPLL_H
