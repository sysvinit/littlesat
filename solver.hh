/* see LICENSE.txt file for license details */

#ifndef SOLVER_HH
#define SOLVER_HH

#include <list>
#include <map>
#include <optional>
#include <utility>

namespace littlesat {
    /* type aliases for SAT formulae in CNF */
    using literal = std::pair<long, bool>;
    using clause = std::map<long, bool>;
    using formula = std::list<clause>;

    using solution = std::map<long, bool>;

    /* main solver function */
    std::optional<solution> dpll(formula&);
}

#endif /* SOLVER_HH */
