#pragma once

#include <iostream>
#include <map>
#include <string>
#include <string_view>
#include <vector>
#include <variant>
#include <optional>
#include <execution>
#include <unordered_set>

namespace json {
    class Node;
    using Dict = std::map<std::string, Node>;
    using Array = std::vector<Node>;

    class ParsingError : public std::runtime_error {
    public:
        using runtime_error::runtime_error;
    };

    class Node : private std::variant<std::nullptr_t, int, double, bool, std::string, Dict, Array> {
    public:
        using variant::variant;
        using Value = variant;
        Node(const std::string_view& str):variant(std::string(str)) {}
        bool IsInt() const;
        bool IsDouble() const;
        bool IsPureDouble() const;
        bool IsBool() const;
        bool IsString() const;
        bool IsNull() const;
        bool IsArray() const;
        bool IsMap() const;

        const Array& AsArray() const;
        const Dict& AsMap() const;
        int AsInt() const;
        const std::string& AsString() const;
        bool AsBool() const;
        double AsDouble() const;        
        bool Swap(Value&& val);
        bool AddValue(Value&& val);
        bool AddValue(std::string key, Value&& val);

        const Value& GetValue() const {
            return *this;
        }

        bool operator==(const Node& rhs) const {
            return GetValue() == rhs.GetValue();
        }        
    };

    inline bool operator!=(const Node& lhs, const Node& rhs) {
        return !(lhs == rhs);
    }

    class Document {
    public:
        Document(Node root);

        const Node& GetRoot() const;

    private:
        Node root_;
    };

    inline bool operator==(const Document& lhs, const Document& rhs) {
        return lhs.GetRoot() == rhs.GetRoot();
    }

    inline bool operator!=(const Document& lhs, const Document& rhs) {
        return !(lhs == rhs);
    }

    Document Load(std::istream& input);

    void PrintArray(const Array& container, std::ostream& output);
    void PrintMap(const Dict& container, std::ostream& output);
    void PrintString(const std::string& str, std::ostream& output);
    void PrintNode(const Node& root, std::ostream& output);
    void Print(const Document& doc, std::ostream& output);

}