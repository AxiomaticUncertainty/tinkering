#include <ostream>

#include <table.hpp>

class Writer {
public:
    virtual ~Writer() = default;

    virtual std::ostream& writeManifest() = 0;
    virtual std::ostream& writeColumn(const std::string& name) = 0;
};

class Serializer {
public:
    Serializer(Writer& writer);

    void write(const Table& table);

private:
    void writeManifest(std::ostream& os, const Table& table);
    struct ColumnWriter {
        ColumnWriter(std::ostream& os) : destination(os) {}

        void operator()(const auto& column) {
            // to be replaced by raw binary
            for (const auto& v : column) {
                destination << v << "\n";
            }
        }

        std::ostream& destination;
    };

    Writer& m_writer;
};