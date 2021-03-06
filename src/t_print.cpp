#include <vector>
#include <string>
#include <iomanip>
#include <iostream>
#include <boost/range/combine.hpp>
#include "t_print.h"


void T_print::print_hor_line ()
{
    std::cout << '+';
    std::cout << std::setw(m_width - 1) << std::setfill('-') << '-';
    std::cout << '+' << std::endl;
}


void T_print::push_header (std::vector<std::string> names, 
        std::vector<uint8_t> width)
{
    if (width.size() == names.size() && width[0] != 0)
    {
        for (auto x : boost::combine(names, width))
        {
            std::string n; uint8_t w;
            boost::tie(n, w) = x;
            m_width += w + 3;
            header.push_back(std::make_pair(n,w));
        }
    } else {
        /*
        for (auto x : names)
        {
            m_width += x.size() + 3;
            header.push_back(std::make_pair(x, x.size()));
        } */
        
        for (size_t i = 0; i < names.size(); ++i)
        {
            m_width += width[i] + 3;
            header.push_back(std::make_pair(names[i], width[i]));
        }
    }
}


void T_print::push_tuple (std::vector<std::string> tup)
{
    if (header.empty() || tup.size() != header.size())
        throw std::invalid_argument("Error while printing table");

    tuples.push_back(tup);
}


void T_print::print()
{
    // printing of header
    print_hor_line();
    std::cout << '|'; 
    for (auto x : header )
    {
        std::cout << ' ' << std::setfill(' ') << std::left
             << std::setw(x.second) << x.first << ' ' << '|';
    }
    std::cout << std::endl;
    print_hor_line();

    // tuples
    for (auto x : tuples)
    {
        std::cout << '|';

        for (int i = 0; i < x.size(); i++)
        {
            std::cout << std::setfill(' ') << std::left <<
                std::setw(header[i].second + 1) << x[i] << " |";
        }

        std::cout << std::endl;
    }

    print_hor_line();
}
