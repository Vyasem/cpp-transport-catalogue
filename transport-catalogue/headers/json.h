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
    // ��������� ���������� Dict � Array ��� ���������
    using Dict = std::map<std::string, Node>;
    using Array = std::vector<Node>;

    // ��� ������ ������ ������������� ��� ������� �������� JSON
    class ParsingError : public std::runtime_error {
    public:
        using runtime_error::runtime_error;
    };

    class Node : private std::variant<std::nullptr_t, int, double, bool, std::string, Dict, Array> {
    public:
        /* ���������� Node, ��������� std::variant */
        using variant::variant;

        bool IsInt() const;
        bool IsDouble() const; //���������� true, ���� � Node �������� int ���� double.
        bool IsPureDouble() const; //���������� true, ���� � Node �������� double.
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
    };

    class Document {
    public:
        Document(Node root);

        const Node& GetRoot() const;

    private:
        Node root_;
    };

    Document Load(std::istream& input);

    void PrintArray(const Array& container, std::ostream& output);
    void PrintMap(const Dict& container, std::ostream& output);
    void PrintString(const std::string& str, std::ostream& output);
    void PrintNode(const Node& root, std::ostream& output);
    void Print(const Document& doc, std::ostream& output);

}  // namespace json