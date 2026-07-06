#include <gtest/gtest.h>

#include <stdexcept>
#include <table.hpp>

// Happy path

// Rows

TEST(Table, RowsEmpty) {
    Schema s;
    Columns c;

    Table t(s, c);

    ASSERT_EQ(t.rows(), 0);
}

TEST(Table, RowsNonEmpty) {
    Schema s = {{"Foo", ColumnType::INT}, {"Bar", ColumnType::STRING}};
    Columns c = {UnderlyingColumn<int>({1, 2, 3}), UnderlyingColumn<std::string>({"tomato", "potato", "eggs"})};

    Table t(s, c);

    ASSERT_EQ(t.rows(), 3);
}

// Schema

TEST(Table, SchemaEmpty) {
    Schema s;
    Columns c;

    Table t(s, c);

    ASSERT_TRUE(t.schema().empty());
}

// Invalid construction

TEST(Table, MissingColumns) {
    Schema s = {{"Foo", ColumnType::INT}, {"Bar", ColumnType::STRING}};
    Columns c = {UnderlyingColumn<int>{1, 2, 3}};

    ASSERT_THROW(Table(s, c), std::invalid_argument);
}

TEST(Table, DuplicateColumnName) {
    Schema s = {{"Foo", ColumnType::INT}, {"Foo", ColumnType::STRING}};
    Columns c = {UnderlyingColumn<int>({1, 2, 3}), UnderlyingColumn<std::string>({"tomato", "potato", "eggs"})};

    ASSERT_THROW(Table(s, c), std::invalid_argument);
}

TEST(Table, SchemaColumnMismatch) {
    Schema s = {{"Foo", ColumnType::INT}};
    Columns c = {UnderlyingColumn<double>{1.0}};

    ASSERT_THROW(Table(s, c), std::invalid_argument);
}

TEST(Table, RowCountMismatch) {
    Schema s = {{"Foo", ColumnType::INT}, {"Bar", ColumnType::STRING}};
    Columns c = {UnderlyingColumn<int>({1, 2, 3}), UnderlyingColumn<std::string>({"tomato", "potato"})};

    ASSERT_THROW(Table(s, c), std::invalid_argument);
}
