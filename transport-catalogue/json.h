#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include<variant>

namespace json {

    class Node;
    using Dict = std::map<std::string, Node>;
    using Array = std::vector<Node>;

    // Эта ошибка должна выбрасываться при ошибках парсинга JSON
    class ParsingError : public std::runtime_error {
    public:
        using runtime_error::runtime_error;
    };

    class Node final : private std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string> {
    public:
        // Делаем доступными все конструкторы родительского класса variant
        using variant::variant;
        using Value = variant;

        Node(Value value) {
            this->swap(value);
        }

        int AsInt() const;
        double AsDouble() const;
        bool AsBool() const;
        const std::string& AsString() const;
        const Array& AsArray() const;
        const Dict& AsMap() const;


        bool IsInt() const;
        bool IsDouble() const;
        bool IsPureDouble() const;
        bool IsBool() const;
        bool IsString() const;
        bool IsNull() const;
        bool IsArray() const;
        bool IsMap() const;

        const variant& GetValue() const;
        Value& GetValue();

        bool operator==(const Node& rhs) const {
            return GetValue() == rhs.GetValue();
        }

    private:
    };

    inline bool operator!=(const Node& lhs, const Node& rhs) {
        return !(lhs == rhs);
    }

    class Document {
    public:
        explicit Document(Node root);

        const Node& GetRoot() const;

        bool operator == (const Document& other) const;
        bool operator != (const Document& other) const;

    private:
        Node root_;
    };

    Document Load(std::istream& input);

    void Print(const Document& doc, std::ostream& output);

}  // namespace json