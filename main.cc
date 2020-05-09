/* see LICENSE.txt file for license details */

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <optional>

#include "solver.hh"
#include "parser.hh"

int main(int argc, char *argv[]) {
    std::ifstream inp;
    littlesat::formula cnf;
    std::optional<littlesat::solution> soln;
    bool stdin;

    stdin = false;

    if (argc == 1) {
        stdin = true;
    } if (argc == 2) {
        inp = std::ifstream(argv[1]);
        if (!inp) {
            std::cerr << "Could not open " << argv[1] << " for reading"
                << std::endl;
            std::exit(EXIT_FAILURE);
        }
    } else if (argc > 2) {
        std::cerr << "Too many arguments" << std::endl;
        std::exit(EXIT_FAILURE);
    }

    if (stdin) {
        littlesat::parse(std::cin, cnf);
    } else {
        littlesat::parse(inp, cnf);
    }

    soln = littlesat::dpll(cnf);

    if (soln) {
        std::cout << "SATISFIABLE" << std::endl;
        for (auto s: soln.value()) {
            std::cout << s.first << ": " << (s.second ? "true" : "false") <<
                std::endl;
        }
    } else {
        std::cout << "UNSATISFIABLE" << std::endl;
    }

    return 0;
}

