#include "headers/json.h"

using namespace std;

namespace json {

    namespace {
        using Number = std::variant<std::nullptr_t, bool, int, double>;

        Node LoadNode(istream& input);

        Node LoadArray(istream& input) {
            Array result;
            char c;
            for (; input >> c && c != ']';) {
                if (c != ',') {
                    input.putback(c);
                }
                result.push_back(LoadNode(input));
            }
            if (c != ']') {
                throw ParsingError("Array parsing error");
            }
            return Node(move(result));
        }

        Node LoadString(std::istream& input) {
            using namespace std::literals;

            auto it = std::istreambuf_iterator<char>(input);
            auto end = std::istreambuf_iterator<char>();
            std::string s;
            while (true) {
                if (it == end) {
                    // Поток закончился до того, как встретили закрывающую кавычку?
                    throw ParsingError("String parsing error");
                }
                const char ch = *it;
                if (ch == '"') {
                    // Встретили закрывающую кавычку
                    ++it;
                    break;
                }
                else if (ch == '\\') {
                    // Встретили начало escape-последовательности
                    ++it;
                    if (it == end) {
                        // Поток завершился сразу после символа обратной косой черты
                        throw ParsingError("String parsing error");
                    }
                    const char escaped_char = *(it);
                    // Обрабатываем одну из последовательностей: \\, \n, \t, \r, \"
                    switch (escaped_char) {
                    case 'n':
                        s.push_back('\n');
                        break;
                    case 't':
                        s.push_back('\t');
                        break;
                    case 'r':
                        s.push_back('\r');
                        break;
                    case '"':
                        s.push_back('"');
                        break;
                    case '\\':
                        s.push_back('\\');
                        break;
                    default:
                        // Встретили неизвестную escape-последовательность
                        throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
                    }
                }
                else if (ch == '\n' || ch == '\r') {
                    // Строковый литерал внутри- JSON не может прерываться символами \r или \n
                    throw ParsingError("Unexpected end of line"s);
                }
                else {
                    // Просто считываем очередной символ и помещаем его в результирующую строку
                    s.push_back(ch);
                }
                ++it;
            }
            return Node(move(s));
        }

        Node LoadDict(istream& input) {
            Dict result;
            char c;
            for (; input >> c && c != '}';) {
                if (c == ',') {
                    input >> c;
                }

                string key = LoadString(input).AsString();
                input >> c;
                result.insert({ move(key), LoadNode(input) });
            }

            if (c != '}') {
                throw ParsingError("Map parsing error");
            }

            return Node(move(result));
        }

        Number LoadNumber(std::istream& input) {
            using namespace std::literals;

            std::string parsed_num;

            if (input.peek() == 'n') {
                std::string str;
                for (int i = 0; i < 4; ++i) {
                    str.push_back(static_cast<char>(input.get()));
                }
                if (str != "null") {
                    throw ParsingError("Expected null");
                }
                return nullptr;
            }

            if (input.peek() == 't') {
                std::string str;
                for (int i = 0; i < 4; ++i) {
                    str.push_back(static_cast<char>(input.get()));
                }
                if (str != "true") {
                    throw ParsingError("Expected boolean true");
                }
                return true;
            }

            if (input.peek() == 'f') {
                std::string str;
                for (int i = 0; i < 5; ++i) {
                    str.push_back(static_cast<char>(input.get()));
                }
                if (str != "false") {
                    throw ParsingError("Expected boolean false");
                }
                return false;
            }

            // Считывает в parsed_num очередной символ из input
            auto read_char = [&parsed_num, &input] {
                parsed_num += static_cast<char>(input.get());
                if (!input) {
                    throw ParsingError("Failed to read number from stream"s);
                }
            };

            // Считывает одну или более цифр в parsed_num из input
            auto read_digits = [&input, read_char] {
                if (!std::isdigit(input.peek())) {
                    throw ParsingError("A digit is expected"s);
                }
                while (std::isdigit(input.peek())) {
                    read_char();
                }
            };

            if (input.peek() == '-') {
                read_char();
            }
            // Парсим целую часть числа
            if (input.peek() == '0') {
                read_char();
                // После 0 в JSON не могут идти другие цифры
            }
            else {
                read_digits();
            }

            bool is_int = true;
            // Парсим дробную часть числа
            if (input.peek() == '.') {
                read_char();
                read_digits();
                is_int = false;
            }

            // Парсим экспоненциальную часть числа
            if (int ch = input.peek(); ch == 'e' || ch == 'E') {
                read_char();
                if (ch = input.peek(); ch == '+' || ch == '-') {
                    read_char();
                }
                read_digits();
                is_int = false;
            }

            try {
                if (is_int) {
                    // Сначала пробуем преобразовать строку в int
                    try {
                        return std::stoi(parsed_num);
                    }
                    catch (...) {
                        // В случае неудачи, например, при переполнении,
                        // код ниже попробует преобразовать строку в double
                    }
                }
                return std::stod(parsed_num);
            }
            catch (...) {
                throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
            }
        }

        Node LoadInt(istream& input) {
            Number result = LoadNumber(input);
            if (std::holds_alternative<int>(result)) {
                return Node(std::get<int>(result));
            }

            if (std::holds_alternative<double>(result)) {
                return Node(std::get<double>(result));
            }

            if (std::holds_alternative<bool>(result)) {
                return Node(std::get<bool>(result));
            }
            return Node(nullptr);
        }

        Node LoadNode(istream& input) {
            char c;
            input >> c;

            if (c == '[') {
                return LoadArray(input);
            }
            else if (c == '{') {
                return LoadDict(input);
            }
            else if (c == '"') {
                return LoadString(input);
            }
            else {
                input.putback(c);
                return LoadInt(input);
            }
        }

    }  // namespace

    bool Node::IsInt() const {
        return std::holds_alternative<int>(*this);
    }

    bool Node::IsDouble() const {
        if (std::holds_alternative<double>(*this)) {
            return true;
        }
        return std::holds_alternative<int>(*this);
    }

    bool Node::IsPureDouble() const {
        return std::holds_alternative<double>(*this);
    }

    bool Node::IsBool() const {
        return std::holds_alternative<bool>(*this);
    }

    bool Node::IsString() const {
        return std::holds_alternative<std::string>(*this);
    }

    bool Node::IsNull() const {
        return std::holds_alternative<std::nullptr_t>(*this);
    }

    bool Node::IsArray() const {
        return std::holds_alternative<Array>(*this);
    }

    bool Node::IsMap() const {
        return std::holds_alternative<Dict>(*this);
    }

    const Array& Node::AsArray() const {
        if (!IsArray()) {
            throw std::logic_error("It is not Array");
        }
        return *std::get_if<Array>(this);
    }

    const Dict& Node::AsMap() const {
        if (!IsMap()) {
            throw std::logic_error("It is not Map");
        }
        return *std::get_if<Dict>(this);
    }

    int Node::AsInt() const {
        if (!IsInt()) {
            throw std::logic_error("It is not Integer");
        }
        return *std::get_if<int>(this);
    }

    const string& Node::AsString() const {
        if (!IsString()) {
            throw std::logic_error("It is not string");
        }
        return *std::get_if<std::string>(this);
    }

    bool Node::AsBool() const {
        if (!IsBool()) {
            throw std::logic_error("It is not boolean");
        }
        return *std::get_if<bool>(this);
    }

    double Node::AsDouble() const {
        if (!IsPureDouble() && !IsInt()) {
            throw std::logic_error("It is not dooble");
        }

        if (IsPureDouble()) {
            return *std::get_if<double>(this);
        }

        return *std::get_if<int>(this);
    }

    Document::Document(Node root)
        : root_(move(root)) {
    }

    const Node& Document::GetRoot() const {
        return root_;
    }

    Document Load(istream& input) {
        return Document{ LoadNode(input) };
    }

    void PrintArray(const Array& container, std::ostream& output) {
        output << "[";
        bool first = true;
        for (const Node& node : container) {
            if (!first) {
                output << ", ";
            }
            PrintNode(node, output);
            first = false;
        }
        output << "]";
    }

    void PrintMap(const Dict& container, std::ostream& output) {
        output << "{";
        bool first = true;
        for (const auto& [key, value] : container) {
            if (!first) {
                output << ", ";
            }
            output << "\"" << key << "\" : ";
            PrintNode(value, output);
            first = false;
        }
        output << "}";
    }

    void PrintString(const std::string& str, std::ostream& output) {
        std::unordered_set<char> specseq{ '\\', '\'', '"' };
        for (const char ch : str) {
            if (ch == '\r') {
                output << "\\r";
                continue;
            }
            if (ch == '\n') {
                output << "\\n";
                continue;
            }
            if (specseq.count(ch)) {
                output << '\\';
            }
            output << ch;
        }
    }

    void PrintNode(const Node& root, std::ostream& output) {
        if (root.IsInt()) {
            output << root.AsInt();
        }
        else if (root.IsDouble()) {
            output << root.AsDouble();
        }
        else if (root.IsString()) {
            output << "\"";
            PrintString(root.AsString(), output);
            output << "\"";
        }
        else if (root.IsNull()) {
            output << "null";
        }
        else if (root.IsBool()) {
            if (root.AsBool()) {
                output << "true";
            }
            else {
                output << "false";
            }
        }
        else if (root.IsArray()) {
            PrintArray(root.AsArray(), output);
        }
        else if (root.IsMap()) {
            PrintMap(root.AsMap(), output);
        }
    }

    void Print(const Document& doc, std::ostream& output) {
        PrintNode(doc.GetRoot(), output);
    }


}  // namespace json