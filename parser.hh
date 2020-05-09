/* see LICENSE.txt file for license details */

#ifndef PARSER_HH
#define PARSER_HH

#include <istream>

#include "solver.hh"

namespace littlesat {
    void parse(std::istream&, formula&);
}

#endif /* PARSER_HH */
