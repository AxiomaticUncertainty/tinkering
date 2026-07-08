#include <cstddef>
#include <ios>
#include <reader.hpp>
#include <stdexcept>
#include <table.hpp>

Deserializer::Deserializer(Reader& reader) : m_reader(reader) {}

Table Deserializer::read() {
    auto [schema, rowCount] = readManifest(m_reader.readManifest());
    
    Columns columns;
    for (std::size_t i = 0; i < schema.size(); ++i) {
        std::istream& columnSource = m_reader.readColumn(schema[i].name);
        auto column = constructColumn(schema[i].type);

        std::visit(ColumnReader(columnSource, rowCount), column);

        columns.push_back(std::move(column));
    }

    return Table(std::move(schema), std::move(columns));
}

std::pair<Schema, std::size_t> Deserializer::readManifest(std::istream& is) {
    is >> std::noskipws;

    std::size_t rowCount, columns;
    char break1, break2;
    is >> rowCount >> break1 >> columns >> break2;

    if (break1 != '\n' || break2 != '\n') {
        throw std::logic_error("Invalid manifest format");
    }

    Schema schema;
    schema.reserve(columns);
    for (std::size_t i = 0; i < columns; ++i) {
        std::string name, type;
        is >> name >> break1 >> type >> break2;

        if (break1 != '\n' || break2 != '\n') {
            throw std::logic_error("Invalid manifest format");
        }

        schema.emplace_back(name, stringToColumnType(type));
    }

    if (schema.size() != columns) {
        throw std::logic_error("Manifest column heading doesn't match parsed columns");
    }

    return {std::move(schema), rowCount};
}