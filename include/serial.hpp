// primitives for writing/reading tables to/from files
#include <istream>
#include <ostream>

#include <table.hpp>

template <class T>
std::ostream& serialize(std::ostream& os, const std::vector<T>& column) {
    for (const auto& v : column) {
        os << v << "\n";
    }

    return os;
}

std::ostream& serialize(std::ostream& os, const Column& column);
std::ostream& serialize(std::ostream& os, const Table& table);

std::istream& deserialize(std::istream& is, Table& table);