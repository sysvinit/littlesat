/* see LICENSE.txt file for license details */

#include <atomic>
#include <future>
#include <optional>

#include "solver.hh"
#include "concurrency.hh"

namespace littlesat {
    static std::atomic<long> num_branches(0);
    static std::atomic<bool> solution_found(false);

    /* propagate a literal */
    static formula propagate(literal& lit, formula& cnf) {
        formula cnf1 = cnf;

        for (auto cnf_iter = cnf1.begin(); cnf_iter != cnf1.end(); ) {
            clause& cl = *cnf_iter; 

            /* check if a given clause contains a literal or its negation */
            auto clause_iter = cl.find(lit.first);

            if (clause_iter == cl.end()) {
                /* not found, go to next clause */
                cnf_iter++;
            } else if (clause_iter->second != lit.second) {
                /* we found the negation; remove the literal from this clause, and
                   go to the next one */
                cl.erase(clause_iter);
                cnf_iter++;
            } else {
                /* we found the literal with the same polarity, so remove the
                   clause entirely */
                cnf_iter = cnf1.erase(cnf_iter);
            }
        }

        return cnf1;
    }

    /* propagate the given literal with its current truth assignment, and then recurse */
    static std::optional<solution> dpll_recurse_once(formula& cnf, literal& lit) {
        formula cnf_prop;
        std::optional<solution> soln;

        cnf_prop = propagate(lit, cnf);
        soln = dpll(cnf_prop);

        if (soln) {
            soln.value()[lit.first] = lit.second;
        }

        return soln;
    }

    /* attempt propagation of the given literal, first assuming its current
       value to be true and then, if that fails, its negation */
    static std::optional<solution> dpll_recurse_twice(formula& cnf, literal& lit) {
        std::optional<solution> soln;

        soln = dpll_recurse_once(cnf, lit);

        if (soln) {
            return soln;
        } else {
            lit.second = !lit.second;
            return dpll_recurse_once(cnf, lit);
        }
    }

    /* attempt propagation of both possible values for a given literal in
       separate threads. */
    static std::optional<solution> dpll_recurse_par(formula& cnf, literal& lit) {
        std::future<std::optional<solution>> fut1, fut2;
        std::optional<solution> soln1, soln2;
        literal lit1, lit2;

        lit1 = lit;
        lit2 = lit;
        lit2.second = !lit2.second;

        fut1 = std::async(
            [&cnf, &lit1]() {
                return dpll_recurse_once(cnf, lit1);
            }
        );

        fut2 = std::async(
            [&cnf, &lit2]() {
                return dpll_recurse_once(cnf, lit2);
            }
        );

        soln1 = fut1.get();
        soln2 = fut2.get();

        return soln1 ? soln1 : soln2;
    }

    /* the DPLL algorithm */
    std::optional<solution> dpll(formula& cnf) {
        std::optional<solution> soln, soln1, soln2;
        std::future<std::optional<solution>> fut1, fut2;
        literal unit;
        formula cnf1, cnf2;
        bool contains_unit, done;
        long level;

        /* check if another thread has found a solution already, and if so
           return false */
        done = std::atomic_load(&solution_found);
        if (done) {
            return std::optional<solution>();
        }

        contains_unit = false;

        /* if we have an empty set of clauses then we have found a satisfying
           assignment */
        if (cnf.size() == 0) {
            /* make other threads aware of the solution */
            std::atomic_store(&solution_found, true);

            return std::optional(solution());
        }

        for (auto cnf_iter: cnf) {
            if (cnf_iter.size() == 0) {
                /* empty clause, so formula is unsatisfiable */
                return std::optional<solution>();
            } else if (cnf_iter.size() == 1) {
                /* we've found a unit literal which can be propagated */
                contains_unit = true;
                unit = *cnf_iter.begin();
            }
        }

        if (contains_unit) {
            /* there's a unit clause, so propagate it and recurse */
            return dpll_recurse_once(cnf, unit);
        } else {
            /* no units found, so find the first literal in our first clause,
               and attempt to solve under assumption. recursion strategy is
               selected based on number of existing active branches. */

            unit = *cnf.begin()->begin();

            level = std::atomic_fetch_add(&num_branches, 1L);

            if (level > branch_threshold) {
                std::atomic_fetch_sub(&num_branches, 1L);
                soln = dpll_recurse_twice(cnf, unit);
            } else {
                soln = dpll_recurse_par(cnf, unit);
                std::atomic_fetch_sub(&num_branches, 1L);
            }

            return soln;
        }
    }
}

