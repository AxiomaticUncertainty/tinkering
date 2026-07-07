#include <stdexcept>

#include <gtest/gtest.h>

#include <reader.hpp>
#include <table.hpp>
#include <writer.hpp>

namespace {

class MemoryStore : public Writer, public Reader {
public:
    virtual std::ostream& writeManifest() override {
        return m_manifest;
    }

    virtual std::ostream& writeColumn(const std::string& name) override {
        m_lookup[name] = m_columns.size();
        return m_columns.emplace_back(name, std::stringstream()).second;
    }

    virtual std::istream& readManifest() override {
        return m_manifest;
    }

    virtual std::istream& readColumn(const std::string& name) override {
        return m_columns[m_lookup.at(name)].second;
    }

private:
    std::stringstream m_manifest;
    std::vector<std::pair<std::string, std::stringstream>> m_columns;
    std::unordered_map<std::string, std::size_t> m_lookup;
};

}

class Serialization : public ::testing::Test {
protected:
    Table roundTrip(const Table& table) {
        MemoryStore store;
        Serializer(store).write(table);
        return Deserializer(store).read();
    }
};

// Table-isolated tests

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

// Serialization round-trip tests

// Trivial end-to-end success

TEST_F(Serialization, Empty) {
    Schema s;
    Columns c;
    Table in(s, c);

    Table out = roundTrip(in);
    
    ASSERT_EQ(in, out);
}

TEST_F(Serialization, NoRows) {
    Schema s{{"Field", ColumnType::STRING}, {"Value", ColumnType::INT}};
    Columns c{UnderlyingColumn<std::string>{}, UnderlyingColumn<int>{}};
    Table in(s, c);

    Table out = roundTrip(in);

    ASSERT_EQ(in, out);
}

TEST_F(Serialization, TrivialTypes) {
    Schema s{{"Field", ColumnType::STRING}, {"Value", ColumnType::INT}};
    Columns c{UnderlyingColumn<std::string>{"foo", "bar"}, UnderlyingColumn<int>{2, 1}};
    Table in(s, c);

    Table out = roundTrip(in);

    ASSERT_EQ(in, out);
}

// current limitations

TEST_F(Serialization, NewlineString) {
    Schema s{{"Field", ColumnType::STRING}, {"Value", ColumnType::INT}};
    Columns c{UnderlyingColumn<std::string>{"foo\n", "bar"}, UnderlyingColumn<int>{2, 1}};
    Table in(s, c);

    Table out = roundTrip(in);

    ASSERT_NE(in, out);
}

TEST_F(Serialization, DecimalPrecision) {
    Schema s{{"Value", ColumnType::DOUBLE}};
    Columns c{UnderlyingColumn<double>{1 / 3.0}};
    Table in(s, c);

    Table out = roundTrip(in);

    ASSERT_NE(in, out);
}

TEST_F(Serialization, StringSpace) {
    Schema s{{"Field", ColumnType::STRING}};
    Columns c{UnderlyingColumn<std::string>{"foo bar", "tomato"}};
    Table in(s, c);

    ASSERT_THROW(roundTrip(in), std::logic_error);
}
