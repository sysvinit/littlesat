/* see LICENSE.txt file for license details */

#include <cstdlib>
#include <iostream>
#include <istream>
#include <sstream>

#include "solver.hh"

namespace littlesat {
    static const std::string preamble_delim = " \t";
    static const std::string problem_type = "cnf";

    /* DIMACS CNF specification parsing routines */

    static void parse_err(const std::string& msg) {
        std::cerr << msg << std::endl;
        exit(EXIT_FAILURE);
    }

    static void parse_preamble(std::istream& input, long& numvars, 
                               long& numclauses) {
        std::string data, problem;
        std::string::size_type pstart, pend, vstart, vend, clstart, clend;

        do {
            std::getline(input, data);

            if (input.bad()) {
                parse_err("I/O error while reading file");
            } else if (input.eof()) {
                parse_err("Unexpected end of file");
            } else if (input.fail()) {
                parse_err("Unhandled error when reading file");
            }

            if (data.size() == 0) {
                parse_err("Unexpected empty line");
            }

            if (data[0] != 'c' && data[0] != 'p') {
                parse_err("Malformed preamble line");
            }
        } while (data[0] == 'c');

#define malformed_spec() parse_err("Malformed problem specification")

        pstart = data.find_first_not_of(preamble_delim, 1);
        if (pstart == std::string::npos) {
            malformed_spec();
        }

        pend = data.find_first_of(preamble_delim, pstart);
        if (pend == std::string::npos) {
            malformed_spec();
        }

        vstart = data.find_first_not_of(preamble_delim, pend);
        if (vstart == std::string::npos) {
            malformed_spec();
        }

        vend = data.find_first_of(preamble_delim, vstart);
        if (vend == std::string::npos) {
            malformed_spec();
        }

        clstart = data.find_first_not_of(preamble_delim, vend);
        if (clstart == std::string::npos) {
            malformed_spec();
        }

        /* special case -- don't want trailing data following the last item on the
           line */
        clend = data.find_first_of(preamble_delim, clstart);
        if (clend == std::string::npos) {
            clend = data.size();
        } else if (data.find_first_not_of(preamble_delim, clend) != 
                   std::string::npos) {
            malformed_spec();
        }

        problem = data.substr(pstart, pend - pstart);
        if (problem != problem_type) {
            malformed_spec();
        }

        if ((std::istringstream(data.substr(vstart, vend - vstart)) >>
             numvars).fail()) {
            malformed_spec();
        }

        if ((std::istringstream(data.substr(clstart, clend - clstart))
             >> numclauses).fail()) {
            malformed_spec();
        }

        if (numvars < 1 || numclauses < 1) {
            malformed_spec();
        }

#undef malformed_spec

        return;
    }

    static long get_next(std::istream& input) {
        long ret;

        input >> ret;

        if (input.bad()) {
            parse_err("I/O error while reading file");
        } else if (input.eof()) {
            parse_err("Unexpected end of file");
        } else if (input.fail()) {
            parse_err("Invalid integral value");
        }

        return ret;
    }

    /* parse a DIMACS format CNF file */
    void parse(std::istream& input, littlesat::formula& cnf) {
        long vars, clauses, litval;
        char last;
        littlesat::clause cl;

        parse_preamble(input, vars, clauses);

        for (long i = 0; i < clauses; i++) {
            cl = clause();

            do {
                litval = get_next(input);

                if (litval != 0) {
                    cl[std::abs(litval)] = (litval < 0 ? false : true);
                }
            } while (litval != 0);

            cnf.push_back(move(cl));
        }

        if ((input >> last).good()) {
            parse_err("Unexpected trailing data");
        }

        return;
    }
}

