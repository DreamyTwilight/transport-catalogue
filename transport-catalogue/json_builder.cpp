#include "json_builder.h"

#include <variant>
#include <map>
#include <stdexcept>


using namespace std::literals;

namespace json {

    Builder::Builder(Node root) : root_(std::move(root)) {
    }

    Builder::KeyItemContext Builder::Key(const std::string key) {
        IsGoToBuild();
        if (!nodes_stack_.back()->IsMap()) {
            throw std::logic_error("You can't use Key method."s);
        }

        if (!nodes_stack_.empty()) {
            auto& dict = std::get<Dict>(nodes_stack_.back()->GetValue());
            dict.emplace(key, Node());
            nodes_stack_.push_back(&(dict[key]));
            return Builder::KeyItemContext(ItemContext(*this));
        }
        return Builder::KeyItemContext(ItemContext(*this));
    }

    bool Builder::IsArrayOrDict(Node::Value value) {
        return value.index() == 1 || value.index() == 2;
    }

    void Builder::InsertValueArrayDict(Node::Value value, bool is_start_array_or_dict) {
        if (nodes_stack_.empty()) {
            root_ = std::move(Node(value));
            if (IsArrayOrDict(value) && is_start_array_or_dict) {
                nodes_stack_.push_back(&root_.value());
            }
        }
        else if ((*nodes_stack_.back()).IsNull()) {
            *nodes_stack_.back() = std::move(Node(value));
            if (!IsArrayOrDict(value)) {
                nodes_stack_.pop_back();
            }
        }
        else if ((*nodes_stack_.back()).IsArray()) {
            Array& array = GetLnkCurentArray();
            array.push_back(Node(value));
            if (IsArrayOrDict(value)) {
                nodes_stack_.push_back(&array.back());
            }
        }
        else {
            throw std::logic_error("You can't insert Value here."s);
        }
    }

    Builder::ItemContext Builder::Value(Node::Value value) {
        IsGoToBuild();
        InsertValueArrayDict(value, false);
        if (nodes_stack_.empty()) {
            return Builder::ValueItemContext(ItemContext(*this));
        }
        else if (nodes_stack_.back()->IsMap()) {
            return Builder::DictItemContext(ItemContext(*this));
        }
        else if (nodes_stack_.back()->IsArray()) {
            return Builder::ArrayItemContext(ItemContext(*this));
        }
        else {
            throw std::logic_error("You can't insert Value here."s);
        }
    }

    Builder::DictItemContext Builder::StartDict() {
        IsGoToBuild();
        InsertValueArrayDict(Dict(), true);
        return Builder::DictItemContext(ItemContext(*this));
    }

    Builder::ArrayItemContext Builder::StartArray() {
        IsGoToBuild();
        InsertValueArrayDict(Array(), true);
        return Builder::ArrayItemContext(ItemContext(*this));
    }

    Builder& Builder::EndDict() {
        IsGoToBuild();
        if (nodes_stack_.back()->IsArray()) {
            throw std::logic_error("You must to use EndDict method."s);
        }
        if ((*nodes_stack_.back()).IsMap()) {
            nodes_stack_.pop_back();
        }
        return *this;
    }

    Builder& Builder::EndArray() {
        IsGoToBuild();
        if (nodes_stack_.back()->IsMap()) {
            throw std::logic_error("You must to use EndArray method."s);
        }
        if ((*nodes_stack_.back()).IsArray()) {
            nodes_stack_.pop_back();
        }
        return *this;
    }

    Node Builder::Build() {
        if (!nodes_stack_.empty() || !root_) {
            throw std::logic_error("You can't build json."s);
        }
        return root_.value();
    }

    Array& Builder::GetLnkCurentArray() {
        auto& array = std::get<Array>(nodes_stack_.back()->GetValue());
        return array;
    }

    void Builder::IsGoToBuild() {
        if (nodes_stack_.empty() && root_) {
            throw std::logic_error("You must to use Build method."s);
        }
    }
} //json::
