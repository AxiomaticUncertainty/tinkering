#include <iostream>
#include <ostream>
#include <sstream>

#include <reader.hpp>
#include <table.hpp>
#include <writer.hpp>

int main() {
    Schema s = {{"Foo", ColumnType::INT}, {"Bar", ColumnType::STRING}, {"Zap", ColumnType::DOUBLE}};
    Columns c = {
        UnderlyingColumn<int>({1, 2, 3}),
        UnderlyingColumn<std::string>({"tomato", "potato", "eggs"}),
        UnderlyingColumn<double>{1.25, 1.66, 0.55}
    };
    Table t(s, c);

    std::cout << t;

    return 0;
}