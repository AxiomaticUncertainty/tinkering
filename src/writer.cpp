#include <iostream>

#include <table.hpp>
#include <writer.hpp>

Serializer::Serializer(Writer& writer) : m_writer(writer) {}

void Serializer::write(const Table& table) {
    auto& manifestDestination = m_writer.writeManifest();
    writeManifest(manifestDestination, table);
    for (std::size_t i = 0; i < table.m_schema.size(); ++i) {
        auto& columnDestination = m_writer.writeColumn(table.m_schema[i].name);
        std::visit(ColumnWriter(columnDestination), table.m_columns[i]);
    }
}

void Serializer::writeManifest(std::ostream& os, const Table& table) {
    os << table.rows() << "\n" << table.schema().size() << "\n";
    for (const auto& column : table.schema()) {
        os << column.name << "\n" << columnTypeToString(column.type) << "\n";
    }
}