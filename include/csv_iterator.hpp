#ifndef _CSV_ITERATOR_HPP_
#define _CSV_ITERATOR_HPP_

#include <string>
#include <istream>
#include <stdexcept>
#include <boost/tuple/tuple.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/tokenizer.hpp>
#include "details/csv_helpers.hpp"

namespace csv {
    template<class Tuple>
        class iterator {
            private:
                bool m_bad;
                std::istream* m_in;
                std::string currentLine;
                boost::escaped_list_separator<char> separator;
                bool m_currentDirty; // Marks that currentResult is no longer valid
                Tuple currentResult; // Useful for passing pointers around
            public:
                /**
                 * Constructor denoting end of range
                 */
                iterator() : m_bad(true),m_currentDirty(true) {
                }

                iterator(std::istream& in):iterator(in, boost::escaped_list_separator<char>())
                {}

                iterator(std::istream& in, const boost::escaped_list_separator<char>& separator )
                    : m_bad(false), m_in(&in),separator(separator), m_currentDirty(true) 
                {
                    this->operator++();
                }

                Tuple const* operator->(){
                    if(m_currentDirty){
                        this->operator*();
                    }
                    return &currentResult;
                }

                Tuple operator*() throw (boost::bad_lexical_cast,std::out_of_range) {
                    if(m_currentDirty){
                        try {
                            using namespace boost;
                            typedef tokenizer< escaped_list_separator<char> > Tokens;
                            Tokens tokens(currentLine, separator);
                            details::filler<Tuple>::fill(currentResult,tokens.begin(),tokens.end());
                        } catch (boost::bad_lexical_cast& ex){
                            m_bad = true;
                            throw(ex);
                        } catch (std::out_of_range& ex){
                            // Exception when not enough columns in the record
                            m_bad = true;
                            throw(ex);
                        }
                        m_currentDirty = false;
                    }
                    return currentResult;
                }

                bool operator==(const iterator& other) {
                    return (m_bad == true) &&  (other.m_bad == true);
                }

                bool operator!=(const iterator& other) {
                    return !(*this == other);
                }

                iterator& operator++() {
                    bool res = (bool)std::getline(*m_in, currentLine);
                    m_bad = !res;
                    m_currentDirty = true;
                    return *this;
                }
        };
};


#endif
