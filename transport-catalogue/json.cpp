#include "json.h"

using namespace std;

namespace json {

    namespace {

        class ParsingError : public std::runtime_error {
        public:
            using runtime_error::runtime_error;
        };

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
                throw json::ParsingError("Failed to load Array from string"s);
            }
            return Node(move(result));
        }

        using Number = std::variant<int, double>;

        Number LoadNumber(std::istream& input) {
            using namespace std::literals;

            std::string parsed_num;

            // Считывает в parsed_num очередной символ из input
            auto read_char = [&parsed_num, &input] {
                parsed_num += static_cast<char>(input.get());
                if (!input) {
                    throw json::ParsingError("Failed to read number from stream"s);
                }
                };

            // Считывает одну или более цифр в parsed_num из input
            auto read_digits = [&input, read_char] {
                if (!std::isdigit(input.peek())) {
                    throw json::ParsingError("A digit is expected"s);
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
                throw json::ParsingError("Failed to convert "s + parsed_num + " to number"s);
            }
        }

        Node LoadInt(istream& input) {
            Number number_in = LoadNumber(input);
            if (int* result = std::get_if<int>(&number_in)) {
                return Node(*result);
            }
            else {
                return Node(std::get<double>(number_in));
            }
        }

        // Считывает содержимое строкового литерала JSON-документа
        // Функцию следует использовать после считывания открывающего символа ":
        Node LoadString(std::istream& input) {
            using namespace std::literals;
            auto it = std::istreambuf_iterator<char>(input);
            auto end = std::istreambuf_iterator<char>();
            std::string s;
            while (true) {
                if (it == end) {
                    // Поток закончился до того, как встретили закрывающую кавычку?
                    throw json::ParsingError("String parsing error");
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
            if (c == '}') {
                return Node(move(result));
            }
            else {
                throw json::ParsingError("Failed to load Dict"s);
            }
        }

        char GetChar(istream& input) {
            if (input.peek() != -1) {
                return input.get();
            }
            else {
                throw json::ParsingError("LoadNode parsing error"s);
            }
        }

        bool IsNotValidChar(const char& c) {
            return (c == '\n') || (c == '\r') || (c == '\t') || (c == ' ');
        }

        Node LoadNull(istream& input) {
            std::string s = "n"s;
            char c;
            for (int i = 0; i < 3; ++i) {
                c = GetChar(input);
                if (c == ',' || c == '}' || c == ']') {
                    input.putback(c);
                    break;
                }
                s += c;
            }
            while (input.peek() != -1) {
                c = input.get();
                if (!IsNotValidChar(c)) {
                    if (c == ',' || c == '}' || c == ']') {
                        input.putback(c);
                        break;
                    }
                    s += c;
                }
            }
            if (s == "null"s) {
                return Node();
            }
            else {
                throw json::ParsingError("LoadNode parsing NULL error"s);
            }
        }

        Node LoadBool(istream& input) {
            char c = GetChar(input);
            std::string s;
            s += c;
            int lenght_word = 3;
            if (c == 'f') {
                lenght_word = 4;
            }
            for (int i = 0; i < lenght_word; ++i) {
                c = GetChar(input);
                if (c == ',' || c == '}' || c == ']') {
                    input.putback(c);
                    break;
                }
                s += c;
            }
            while (input.peek() != -1) {
                c = input.get();
                if (!IsNotValidChar(c)) {
                    if (c == ',' || c == '}' || c == ']') {
                        input.putback(c);
                        break;
                    }
                    s += c;
                }
            }
            if (s == "true"s) {
                return Node(true);
            }
            else if (s == "false"s) {
                return Node(false);
            }
            else {
                throw json::ParsingError("LoadNode parsing NULL error"s);
            }
        }

        Node LoadNode(istream& input) {
            char c = GetChar(input);
            while (IsNotValidChar(c)) {
                c = GetChar(input);
            }
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
                if (c == 'n') {
                    return LoadNull(input);
                }
                else if (c == 'f' || c == 't') {
                    input.putback(c);
                    return LoadBool(input);
                }
                else {
                    if (c == '-' || ((c >= '0') && (c <= '9'))) {
                        input.putback(c);
                        return LoadInt(input);
                    }
                    else {
                        throw json::ParsingError("LoadNode input number error"s);
                    }
                }
            }
        }
    }  // namespace

    // std::get_if вернёт указатель на значение нужного типа 
    // либо nullptr, если variant содержит значение другого типа.
    bool Node::IsInt() const {
        return std::get_if<int>(this);
    }
    // Возвращает true, если в Node хранится int либо double
    bool Node::IsDouble() const {
        return std::get_if<double>(this) || std::get_if<int>(this);
    }
    // Возвращает true, если в Node хранится double
    bool Node::IsPureDouble() const {
        return std::get_if<double>(this);
    }
    bool Node::IsBool() const {
        return std::get_if<bool>(this);
    }
    bool Node::IsString() const {
        return std::get_if<std::string>(this);
    }
    bool Node::IsNull() const {
        return std::get_if<std::nullptr_t>(this);
    }
    bool Node::IsArray() const {
        return std::get_if<json::Array>(this);
    }
    bool Node::IsMap() const {
        return std::get_if<json::Dict>(this);
    }

    const Array& Node::AsArray() const {
        return IsArray() ? std::get<json::Array>(*this) : throw std::logic_error("Logic error. Not found <json::Array> in Node."s);
    }
    const Dict& Node::AsMap() const {
        return IsMap() ? std::get<json::Dict>(*this) : throw std::logic_error("Logic error. Not found <json::Dict> in Node."s);
    }
    int Node::AsInt() const {
        return IsInt() ? std::get<int>(*this) : throw std::logic_error("Logic error. Not found <int> in Node."s);
    }
    bool Node::AsBool() const {
        return IsBool() ? std::get<bool>(*this) : throw std::logic_error("Logic error. Not found <bool> in Node."s);
    }
    double Node::AsDouble() const {
        if (!IsDouble()) {
            throw std::logic_error("Logic error. Not found <double> or <int> in Node."s);
        }
        if (IsInt()) {
            return AsInt() * 1.0;
        }
        return std::get<double>(*this);
    }
    
    const string& Node::AsString() const {
        return IsString() ? std::get<std::string>(*this) : throw std::logic_error("Logic error. Not found <std::string> in Node."s);
    }

    const  Node::variant& Node::GetValue() const {
        return *this;
    }

    Node::Value& Node::GetValue() {
        return *this;
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

    bool Document::operator == (const Document& other) const {
        return this->GetRoot().GetValue() == other.GetRoot().GetValue();
    }
    bool Document::operator != (const Document& other) const {
        return !(this->GetRoot().GetValue() == other.GetRoot().GetValue());
    }

    struct PrintContext {
        std::ostream& out;
        int indent_step = 4;
        int indent = 0;

        void PrintIndent() const {
            for (int i = 0; i < indent; ++i) {
                out.put(' ');
            }
        }

        PrintContext Indented() const {
            return { out, indent_step, indent_step + indent };
        }
    };

    void PrintNode(const Node& value, const PrintContext& ctx);

    template <typename Value>
    void PrintValue(const Value& value, const PrintContext& ctx) {
        ctx.out << value;
    }

    void PrintString(const std::string& value, std::ostream& out) {
        out.put('"');
        for (const char c : value) {
            switch (c) {
            case '\r':
                out << "\\r"sv;
                break;
            case '\n':
                out << "\\n"sv;
                break;
            case '\t':
                out << "\\t"sv;
                break;
            case '"':
                // Символы " и \ выводятся как \" или \\, соответственно
                [[fallthrough]];
            case '\\':
                out.put('\\');
                [[fallthrough]];
            default:
                out.put(c);
                break;
            }
        }
        out.put('"');
    }

    template <>
    void PrintValue<std::string>(const std::string& value, const PrintContext& ctx) {
        PrintString(value, ctx.out);
    }

    template <>
    void PrintValue<std::nullptr_t>(const std::nullptr_t&, const PrintContext& ctx) {
        ctx.out << "null"sv;
    }

    // В специализации шаблона PrintValue для типа bool параметр value передаётся
    // по константной ссылке, как и в основном шаблоне.
    // В качестве альтернативы можно использовать перегрузку:
    // void PrintValue(bool value, const PrintContext& ctx);
    template <>
    void PrintValue<bool>(const bool& value, const PrintContext& ctx) {
        ctx.out << (value ? "true"sv : "false"sv);
    }

    template <>
    void PrintValue<Array>(const Array& nodes, const PrintContext& ctx) {
        std::ostream& out = ctx.out;
        out << "[\n"sv;
        bool first = true;
        auto inner_ctx = ctx.Indented();
        for (const Node& node : nodes) {
            if (first) {
                first = false;
            }
            else {
                out << ",\n"sv;
            }
            inner_ctx.PrintIndent();
            PrintNode(node, inner_ctx);
        }
        out.put('\n');
        ctx.PrintIndent();
        out.put(']');
    }

    template <>
    void PrintValue<Dict>(const Dict& nodes, const PrintContext& ctx) {
        std::ostream& out = ctx.out;
        out << "{\n"sv;
        bool first = true;
        auto inner_ctx = ctx.Indented();
        for (const auto& [key, node] : nodes) {
            if (first) {
                first = false;
            }
            else {
                out << ",\n"sv;
            }
            inner_ctx.PrintIndent();
            PrintString(key, ctx.out);
            out << ": "sv;
            PrintNode(node, inner_ctx);
        }
        out.put('\n');
        ctx.PrintIndent();
        out.put('}');
    }

    void PrintNode(const Node& node, const PrintContext& ctx) {
        std::visit(
            [&ctx](const auto& value) {
                PrintValue(value, ctx);
            },
            node.GetValue());
    }

    void Print(const Document& doc, std::ostream& output) {
        PrintNode(doc.GetRoot(), PrintContext{ output });
    }

}  // namespace json