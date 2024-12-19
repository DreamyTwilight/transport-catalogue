#pragma once

#include<optional>

#include "json.h"

namespace json {

    class Builder {

        class ItemContext;
        class DictItemContext;
        class ArrayItemContext;
        class KeyItemContext;
        class ValueItemContext;

    public:

        Builder() = default;

        Builder(Node root);

        KeyItemContext Key(const std::string key);
        ItemContext Value(Node::Value value);
        DictItemContext StartDict();
        ArrayItemContext StartArray();
        Builder& EndDict();
        Builder& EndArray();
        Node Build();

    private:

        class ItemContext {
        public:
            ItemContext(Builder& builder) : builder_(builder) {}

            KeyItemContext Key(const std::string key) {
                return builder_.Key(key);
            }
            ItemContext Value(Node::Value value) {
                return builder_.Value(std::move(value));
            }
            DictItemContext StartDict() {
                return builder_.StartDict();
            }
            ArrayItemContext StartArray() {
                return builder_.StartArray();
            }
            Builder& EndDict() {
                return builder_.EndDict();
            }
            Builder& EndArray() {
                return builder_.EndArray();
            }
            Node Build() {
                return builder_.Build();
            }

        private:
            Builder& builder_;
        };

        class DictItemContext : public ItemContext {
        public:
            DictItemContext(ItemContext item_content) :ItemContext(item_content) {}
            ItemContext Value(Node::Value value) = delete;
            DictItemContext StartDict() = delete;
            ArrayItemContext StartArray() = delete;
            Builder& EndArray() = delete;
            Node Build() = delete;
        };

        class ArrayItemContext : public ItemContext {
        public:
            ArrayItemContext(ItemContext item_content) :ItemContext(item_content) {}
            KeyItemContext Key(const std::string key) = delete;
            ArrayItemContext Value(Node::Value value) {
                return  ItemContext::Value(std::move(value));
            }
            Builder& EndDict() = delete;
            Node Build() = delete;
        };

        class KeyItemContext : public ItemContext {
        public:
            KeyItemContext(ItemContext item_content) :ItemContext(item_content) {}
            KeyItemContext Key(const std::string key) = delete;
            DictItemContext Value(Node::Value value) {
                return  ItemContext::Value(std::move(value));
            }
            Builder& EndDict() = delete;
            Builder& EndArray() = delete;
            Node Build() = delete;
        };

        class ValueItemContext : public ItemContext {
        public:
            ValueItemContext(ItemContext item_content) :ItemContext(item_content) {}
            KeyItemContext Key(const std::string key) = delete;
            ItemContext Value(Node::Value value) = delete;
            DictItemContext StartDict() = delete;
            ArrayItemContext StartArray() = delete;
            Builder& EndDict() = delete;
            Builder& EndArray() = delete;
        };

        std::optional<Node> root_;
        std::vector<Node*> nodes_stack_;

        Array& GetLnkCurentArray();
        void IsGoToBuild();
        void InsertValueArrayDict(Node::Value value, bool is_start_array_or_dict);
        bool IsArrayOrDict(Node::Value value);

    };
} //json::


