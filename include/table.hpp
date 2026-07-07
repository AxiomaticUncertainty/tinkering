#pragma once

#include <concepts>
#include <iostream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <variant>
#include <vector>

class Serializer;

enum class ColumnType {
    INT,
    DOUBLE,
    STRING
};

inline std::string columnTypeToString(ColumnType columnType) {
    switch (columnType) {
    case ColumnType::INT:
        return "int";
    case ColumnType::DOUBLE:
        return "double";
    case ColumnType::STRING:
        return "string";
    default:
        throw std::invalid_argument("Invalid columnType");
    }
}

inline ColumnType stringToColumnType(const std::string& type) {
    if (type == "int") {
        return ColumnType::INT;
    } else if (type == "double") {
        return ColumnType::DOUBLE;
    } else if (type == "string") {
        return ColumnType::STRING;
    }

    throw std::invalid_argument("Can't parse column type " + type);
}

//------------------------------------------------------------------------------
// Type list
//------------------------------------------------------------------------------

template <class... Ts>
struct type_list {};

template <class T, ColumnType C> struct column_type_def;

using column_type_defs = type_list<
    column_type_def<int, ColumnType::INT>,
    column_type_def<double, ColumnType::DOUBLE>,
    column_type_def<std::string, ColumnType::STRING>
>;

template <class T> struct unwrap_column_type;

template <class T, ColumnType C>
struct unwrap_column_type<column_type_def<T, C>> {
    using value = T;
};

template <class T>
using unwrapped_column_t = typename unwrap_column_type<T>::value;

template <class List>
struct unwrap_column_types;

template <class... Defs>
struct unwrap_column_types<type_list<Defs...>> {
    using type = type_list<unwrapped_column_t<Defs>...>;
};

template <class List>
using unwrap_column_types_t = typename unwrap_column_types<List>::type;

using unwrapped_column_types = unwrap_column_types_t<column_type_defs>;

//------------------------------------------------------------------------------
// contains<T, List>
//------------------------------------------------------------------------------

template <class T, class List>
struct contains;

template <class T, class... Ts>
struct contains<T, type_list<Ts...>>
    : std::bool_constant<(std::same_as<T, Ts> || ...)> {};

//------------------------------------------------------------------------------
// Concept
//------------------------------------------------------------------------------

template <class T>
concept UnderlyingColumnType = contains<T, unwrapped_column_types>::value;

//------------------------------------------------------------------------------
// Column storage
//------------------------------------------------------------------------------

template <UnderlyingColumnType T>
using UnderlyingColumn = std::vector<T>;

template <class List>
struct make_column_variant;

template <class... Ts>
struct make_column_variant<type_list<Ts...>> {
    using type = std::variant<UnderlyingColumn<Ts>...>;
};

using Column = make_column_variant<unwrapped_column_types>::type;
using Columns = std::vector<Column>;

Column constructColumn(ColumnType type);

// Table

struct ColumnDefinition {
    std::string name;
    ColumnType type;

    bool operator==(const ColumnDefinition& other) const;
};

using Schema = std::vector<ColumnDefinition>;

class Table {
public:
    Table() = default;
    Table(Schema schema, Columns columns);

    bool operator==(const Table& other) const;

    const Schema& schema() const;
    std::size_t rows() const;

private:
    Schema m_schema;
    Columns m_columns;
    std::unordered_map<std::string, std::size_t> m_columnIndices;

    void ensureValid();
    std::size_t rows(const auto& column) const;

    friend std::ostream& operator<<(std::ostream&, const Table&);
    friend std::ostream& serialize(std::ostream& os, const Table& table);
    friend std::istream& deserialize(std::istream& is, Table& table);
    friend Serializer;
};

std::size_t Table::rows(const auto& column) const {
    auto visitor = [](const auto& column) {
        return column.size();
    };

    return std::visit(visitor, column);
}

std::ostream& operator<<(std::ostream& os, const Table& table); // redundant