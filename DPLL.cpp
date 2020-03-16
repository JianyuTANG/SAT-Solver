//
// DPLL algorithm.
//

#include "DPLL.h"
#include <time.h>
#include <iostream>


int decision_time = 0;

literal DPLL::propagate_noncdcl()
{
	bool exist_unit = true;
	while (exist_unit)
	{
		exist_unit = false;
		for (const auto& c : phi.clauses)
		{
			int remaining = 0, index = 0;
			for (int l : c)
			{
				if (mp[VAR(l)] == 0)
				{
					remaining++;
					if (remaining > 1)
						break;
					index = l;
				}
				else if (POSITIVE(l * mp[VAR(l)]))
				{
					remaining = 2;
					break;
				}
			}

			// only one var left
			// able to propagate
			if (remaining == 1)
			{
				int target = index;
				if (POSITIVE(index))
					mp[index] = 1;
				else
				{
					index = 0 - index;
					mp[index] = -1;
				}
				remaining_var--;
				st.push(choice(target, PROPAGATE));
				exist_unit = true;
			}

			// exist conflict
			else if (remaining == 0)
			{
				return 1;
			}
		}
	}
	return 0;
}


literal DPLL::propagate()
{
	bool exist_unit = true;
	while (exist_unit)
	{
		exist_unit = false;
		for (const auto& c : phi.clauses)
		{
			int remaining = 0, index = 0;
			for (int l : c)
			{
				if (mp[VAR(l)] == 0)
				{
					remaining++;
					if (remaining > 1)
						break;
					index = l;
				}
				else if (POSITIVE(l * mp[VAR(l)]))
				{
					remaining = 2;
					break;
				}
			}

			// only one var left
			// able to propagate
			if (remaining == 1)
			{
				int target = index;
				if (POSITIVE(index))
					mp[index] = 1;
				else
				{
					index = 0 - index;
					mp[index] = -1;
				}
				remaining_var--;
				st.push(choice(target, PROPAGATE));
				time_stamp[index] = cur_time++;
				roots[index].clear();
				for (int l : c)
				{
					if (l != target)
					{
						roots[index].insert(roots[VAR(l)].begin(), roots[VAR(l)].end());
					}
				}
				exist_unit = true;
			}

			// exist conflict
			else if (remaining == 0)
			{
				// find the last one propagated
				int latest = 1;
				unsigned long long t = 0;
				for (int l : c)
				{
					int i = VAR(l);
					if (time_stamp[i] > t)
					{
						t = time_stamp[i];
						latest = l;
					}
				}
				// add nodes to its roots
				int cl = VAR(latest);
				for (int l : c)
				{
					if (l != latest)
					{
						int i = VAR(l);
						roots[cl].insert(roots[i].begin(), roots[i].end());
					}
				}
				clause confict_clause;
				for (auto iter = roots[cl].rbegin(); iter != roots[cl].rend(); iter++)
					confict_clause.push_back(-1 * (*iter));
				phi.clauses.push_back(confict_clause);
				return cl;
			}
		}
	}
	return 0;
}

bool DPLL::try_backtrack() {
	while (!st.empty())
	{
		choice c = st.top();
		switch (c.category)
		{
		case DECIDE:
		{
			st.pop();
			c.category = BACKTRACK;
			int v = c.variable;
			c.variable = 0 - v;
			st.push(c);
			int index = VAR(v);
			mp[index] = 0 - mp[index];
			return true;
		}
		case BACKTRACK:
		case PROPAGATE:
			st.pop();
			mp[VAR(c.variable)] = 0;
			remaining_var++;
			break;
		default:
			break;
		}
	}
	return false;
}

bool DPLL::try_backjump() {
	literal l = 0, target = 0;
	clause confict_clause = phi.clauses.back();
	if (confict_clause.size() == 0)
		return false;
	while (!st.empty())
	{
		choice c = st.top();
		switch (c.category)
		{
		case DECIDE:
		{
			if (target == 0)
			{
				target = decision_st.top();
				literal last = target;
				decision_st.pop();
				while (!decision_st.empty())
				{
					literal cur = decision_st.top();
					if (std::binary_search(confict_clause.begin(), confict_clause.end(), cur * (-1)))
					{
						break;
					}
					else
					{
						last = cur;
						decision_st.pop();
					}
				}
				l = last;
			}
			if (c.variable == l)
			{
				st.pop();
				mp[VAR(c.variable)] = 0;
				roots[VAR(c.variable)].clear();

				st.push(choice(-1 * target, BACKTRACK));
				int index = VAR(target);
				if (POSITIVE(target))
					mp[index] = -1;
				else
					mp[index] = 1;
				roots[index].clear();
				time_stamp[index] = cur_time++;
				for (int t : confict_clause)
				{
					roots[index].insert(roots[VAR(t)].begin(), roots[VAR(t)].end());
				}
				return true;
			}
			else
			{
				st.pop();
				mp[VAR(c.variable)] = 0;
				roots[VAR(c.variable)].clear();
				remaining_var++;
			}
			break;
		}
		case BACKTRACK:
		case PROPAGATE:
			st.pop();
			mp[VAR(c.variable)] = 0;
			roots[VAR(c.variable)].clear();
			remaining_var++;
			break;
		default:
			break;
		}
	}
	return false;
}

void DPLL::make_decision_noncdcl(std::default_random_engine& random, std::bernoulli_distribution& d)
{
	int n = phi.num_variable;
	if (remaining_var == 0)
		return;
	for (int i = 1; i <= n; i++)
	{
		if (mp[i] == 0 && visited[i] != 0)
		{
			int temp = i;
			mp[i] = 1;
			bool is_pos = d(random);

			if (is_pos)
			{
				mp[i] = 1;
			}
			else
			{
				mp[i] = -1;
				temp *= -1;
			}
			decision_time++;
			st.push(choice(temp, DECIDE));
			remaining_var--;
			return;
		}
	}
}

void DPLL::make_decision(std::default_random_engine& random, std::bernoulli_distribution& d)
{
	int n = phi.num_variable;
	if (remaining_var == 0)
		return;
	for (int i = 1; i <= n; i++)
	{
		if (mp[i] == 0 && visited[i] != 0)
		{
			int temp = i;
			mp[i] = 1;
			bool is_pos = d(random);
			
			if (is_pos)
			{
				mp[i] = 1;
			}
			else
			{
				mp[i] = -1;
				temp *= -1;
			}
			decision_time++;
			st.push(choice(temp, DECIDE));
			decision_st.push(temp);
			roots[i].clear();
			roots[i].insert(temp);
			time_stamp[i] = cur_time++;
			remaining_var--;
			return;
		}
	}
}

void DPLL::traverse_clauses() {
	int n = phi.clauses.size();
	//std::cout << n << std::endl;
	for (int i = 0; i < n; i++)
	{
		for (int l : phi.clauses[i])
		{
			visited[VAR(l)] = 1;
		}
	}

	for (int i = 1; i <= phi.num_variable; i++)
	{
		if (!visited[i])
			remaining_var--;
	}
}

bool DPLL::check_sat() {
	// init random engine
	std::default_random_engine random(time(NULL));
	std::bernoulli_distribution d(0.5);

	// preprocess
	traverse_clauses();

	// start main loop
	while (1)
	{
		// propagte
		literal conflict_literal = propagate();

		// exist conflict
		while (conflict_literal)
		{
			// backjump
			if (!try_backjump())
			{
				/*std::cout << std::endl << decision_time;*/
				return false;
			}
			conflict_literal = propagate();
		}

		// check if already sat
		if (remaining_var == 0)
			return true;

		// make decision
		make_decision(random, d);
	}
}

bool DPLL::check_sat_noncdcl() {
	// init random engine
	std::default_random_engine random(time(NULL));
	std::bernoulli_distribution d(0.5);

	// preprocess
	traverse_clauses();

	// start main loop
	while (1)
	{
		// propagte
		literal conflict_literal = propagate_noncdcl();

		// exist conflict
		while (conflict_literal)
		{
			// backjump
			if (!try_backtrack())
			{
				/*std::cout << std::endl << decision_time;*/
				return false;
			}
			conflict_literal = propagate_noncdcl();
		}

		// check if already sat
		if (remaining_var == 0)
			return true;

		// make decision
		make_decision_noncdcl(random, d);
	}
}

model DPLL::get_model() {
	model ans;
	for (int i = 1; i <= phi.num_variable; i++)
	{
		if (mp[i] == 1)
		{
			ans[i] = true;
		}
		else
		{
			ans[i] = false;
		}
	}
	return ans;
}