#include <iostream>
#include <istream>
#include <ostream>

#include <serial.hpp>
#include <stdexcept>
#include <table.hpp>

std::ostream& serialize(std::ostream& os, const Column& column) {
    auto visitor = [&](const auto& c) {
        serialize(os, c);
    };

    std::visit(visitor, column);

    return os;
}

std::ostream& serialize(std::ostream& os, const Table& table) {
    // number of columns in schema spec, number of rows in table
    os << table.m_schema.size() << "," << table.rows() << "\n";

    // for each column
    for (std::size_t i = 0; i < table.m_schema.size(); ++i) {
        // column name & type
        os << table.m_schema[i].name << "\n" << columnTypeToString(table.m_schema[i].type) << "\n";
        // column values
        serialize(os, table.m_columns[i]);
    }

    return os;
}

namespace {

std::istream& deserialize(std::istream& is, ColumnDefinition& columnSchema) {
    is >> std::noskipws;
    
    std::string name, type;
    char break1, break2;

    is >> name >> break1 >> type >> break2;
    if (break1 != '\n' || break2 != '\n') {
        throw std::logic_error("Error consuming column heading");
    }

    columnSchema.type = stringToColumnType(type);
    columnSchema.name = name;

    return is;
}

template <class T>
std::istream& deserialize(std::istream& is, UnderlyingColumn<T>& column, std::size_t rows) {
    is >> std::noskipws;
    
    for (std::size_t row = 0; row < rows; ++row) {
        T elem;
        char linebreak;

        is >> elem >> linebreak;

        if (linebreak != '\n') {
            throw std::logic_error("Can't parse column");
        }

        column.push_back(std::move(elem));
    }

    return is;
}

}

std::istream& deserialize(std::istream& is, Table& table) {
    int columnCount, rowCount;
    char comma, linebreak;

    is >> std::noskipws;

    // parse dimensions
    is >> columnCount >> comma >> rowCount >> linebreak;
    if (comma != ',' || linebreak != '\n') {
        throw std::logic_error("Error!");
    }

    table.m_schema.reserve(columnCount);
    table.m_columns.reserve(columnCount);

    auto visitor = [rowCount, &is](auto& column) {
        deserialize(is, column, rowCount);
    };

    for (std::size_t i = 0; i < rowCount; ++i) {
        // build column definition
        auto& def = table.m_schema.emplace_back();
        deserialize(is, def);
        table.m_columns.push_back(constructColumn(def.type));

        // parse column
        Column& column = table.m_columns.back();
        std::visit(visitor, column);
    }

    table.ensureValid();

    return is;
}