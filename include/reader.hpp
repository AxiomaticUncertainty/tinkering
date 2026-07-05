#include <ios>
#include <istream>

#include <stdexcept>
#include <table.hpp>

class Reader {
public:
    virtual ~Reader() = default;

    virtual std::istream& readManifest() = 0;
    virtual std::istream& readColumn(const std::string& name) = 0;
};

class Deserializer {
public:
    Deserializer(Reader& reader);

    Table read();

private:
    std::pair<Schema, std::size_t> readManifest(std::istream& is);
    struct ColumnReader {
        ColumnReader(std::istream& is, std::size_t expectedRows) : source(is), rows(expectedRows) {}

        template <class T>
        void operator()(UnderlyingColumn<T>& column) {
            source >> std::noskipws;

            // to be repalced by raw binary
            for (std::size_t i = 0; i < rows; ++i) {
                T elem;
                char linebreak;

                source >> elem >> linebreak;

                if (linebreak != '\n') {
                    throw std::logic_error("Invalid format for column");
                }

                column.push_back(std::move(elem));
            }
        }

        std::istream& source;
        std::size_t rows;
    };

    Reader& m_reader;
};