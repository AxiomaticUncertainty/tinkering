#include <iomanip>
#include <ostream>
#include <stdexcept>
#include <sstream>
#include <string>

#include <table.hpp>

namespace {

struct Validator {
    Validator(ColumnType columnType) : columnType(columnType) {}

    bool operator()(const UnderlyingColumn<std::string>&) {
        if (columnType != ColumnType::STRING) {
            return false;
        }
    
        return true;
    }

    bool operator()(const UnderlyingColumn<int>&) {
        if (columnType != ColumnType::INT) {
            return false;
        }
    
        return true;
    }

    bool operator()(const UnderlyingColumn<double>&) {
        if (columnType != ColumnType::DOUBLE) {
            return false;
        }
    
        return true;
    }

    ColumnType columnType;
};

}

Column constructColumn(ColumnType type) {
    switch (type) {
    case ColumnType::INT:
        return UnderlyingColumn<int>();
    case ColumnType::DOUBLE:
        return UnderlyingColumn<double>();
    case ColumnType::STRING:
        return UnderlyingColumn<std::string>();
    default:
        throw std::invalid_argument("Bad type for column");
    }
}

bool ColumnDefinition::operator==(const ColumnDefinition& other) const {
    return name == other.name && type == other.type;
}

Table::Table(Schema schema, Columns columns) : m_schema(std::move(schema)), m_columns(std::move(columns)) {
    ensureValid();
}

void Table::ensureValid() {
    m_columnIndices.clear();

    if (m_schema.size() != m_columns.size()) {
        throw std::invalid_argument("Invalid table: cardinality of schema and actual columns not equal!");
    }

    auto expectedRows = rows();
    for (std::size_t i = 0; i < m_schema.size(); ++i) {
        auto [_, inserted] = m_columnIndices.emplace(m_schema[i].name, i);
        if (!inserted) {
            throw std::invalid_argument("Invalid table: duplicate column names!");
        }

        if (!std::visit(Validator(m_schema[i].type), m_columns[i])) {
            throw std::invalid_argument("Invalid table: schema does not match columns!");
        }

        if (rows(m_columns[i]) != expectedRows) {
            throw std::invalid_argument("Invalid table: row counts don't match!");
        }
    }
}

bool Table::operator==(const Table& other) const {
    return m_schema == other.m_schema && m_columns == other.m_columns;
}

const Schema& Table::schema() const {
    return m_schema;
}

std::size_t Table::rows() const {
    if (m_columns.empty()) {
        return 0;
    }

    return rows(m_columns[0]);
}

std::ostream& operator<<(std::ostream& os, const Table& table) {
    std::vector<std::size_t> widths(table.m_schema.size());

    // Start with header widths.
    for (std::size_t col = 0; col < table.m_schema.size(); ++col)
        widths[col] = table.m_schema[col].name.size();

    // Expand widths based on cell contents.
    for (std::size_t col = 0; col < table.m_columns.size(); ++col) {
        std::visit([&](const auto& column) {
            for (const auto& value : column) {
                std::ostringstream ss;
                ss << value;
                widths[col] = std::max(widths[col], ss.str().size());
            }
        }, table.m_columns[col]);
    }

    // Header.
    for (std::size_t col = 0; col < table.m_schema.size(); ++col) {
        if (col)
            os << " | ";
        os << std::left << std::setw(static_cast<int>(widths[col])) << table.m_schema[col].name;
    }
    os << '\n';

    // Separator.
    for (std::size_t col = 0; col < table.m_schema.size(); ++col) {
        if (col)
            os << "-+-";
        os << std::string(widths[col], '-');
    }
    os << '\n';

    // Rows.
    for (std::size_t row = 0; row < table.rows(); ++row) {
        for (std::size_t col = 0; col < table.m_columns.size(); ++col) {
            if (col)
                os << " | ";

            std::visit([&](const auto& column) {
                os << std::left << std::setw(static_cast<int>(widths[col])) << column[row];
            }, table.m_columns[col]);
        }
        os << '\n';
    }

    return os;
}