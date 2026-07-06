#include <iostream>
#include <ostream>
#include <sstream>

#include <reader.hpp>
#include <table.hpp>
#include <writer.hpp>

class SampleDriver : public Writer, public Reader {
public:
    virtual std::ostream& writeManifest() {
        return m_manifest;
    }

    virtual std::ostream& writeColumn(const std::string& name) {
        m_lookup[name] = m_columns.size();
        return m_columns.emplace_back(name, std::stringstream()).second;
    }

    virtual std::istream& readManifest() {
        return m_manifest;
    }

    virtual std::istream& readColumn(const std::string& name) {
        return m_columns[m_lookup.at(name)].second;
    }

private:
    std::stringstream m_manifest;
    std::vector<std::pair<std::string, std::stringstream>> m_columns;
    std::unordered_map<std::string, std::size_t> m_lookup;
};

int main() {
    Schema s = {{"Foo", ColumnType::INT}, {"Bar", ColumnType::STRING}, {"Zap", ColumnType::DOUBLE}};
    Columns c = {
        UnderlyingColumn<int>({1, 2, 3}),
        UnderlyingColumn<std::string>({"tomato", "potato", "eggs"}),
        UnderlyingColumn<double>{1.25, 1.66, 0.55}
    };
    Table t(s, c);

    // std::cout << t << std::endl;
    // std::cout << "\n\n";

    // std::stringstream ss;
    // serialize(ss, t);

    // // std::cout << ss.str() << std::endl;
    // std::istringstream is(ss.str());
    // Table parsed;
    // deserialize(is, parsed);
    // std::cout << "Parsed table:\n" << parsed;
    SampleDriver driver;
    Serializer serializer(driver);
    serializer.write(t);
    Deserializer deserializer(driver);
    std::cout << "Parsed table:\n" << deserializer.read();

    return 0;
}