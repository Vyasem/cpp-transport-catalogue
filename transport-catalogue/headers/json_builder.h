#pragma once

#include <string>
#include <stack>
#include <vector>
#include "json.h"

namespace json {
	class KeyItemContext;
	class SingleValueItemContext;
	class DictItemContext;
	class ArrayItemContext;

	class Builder {
	private:
		Node root_;
		std::stack<Node*> queue_;
	public:
		Builder() = default;
		KeyItemContext Key(std::string key);
		SingleValueItemContext Value(Node::Value value);
		DictItemContext StartDict();
		Builder& EndDict();
		ArrayItemContext StartArray();
		Builder& EndArray();
		Node Build();
	};

	class ItemContext {
	protected:
		Builder& builder_;
	public:
		ItemContext(Builder& builder);
		KeyItemContext Key(std::string key);
		SingleValueItemContext Value(Node::Value value);
		DictItemContext StartDict();
		ArrayItemContext StartArray();
		Builder& EndDict();
		Builder& EndArray();
		Node Build();
	};

	class KeyItemContext : public ItemContext {
	public:
		KeyItemContext(Builder& builder);
		DictItemContext Value(Node::Value value);
		KeyItemContext Key(std::string key) = delete;
		Builder& EndDict() = delete;
		Builder& EndArray() = delete;
		Node Build() = delete;
	};

	class SingleValueItemContext : public ItemContext {
	public:
		SingleValueItemContext(Builder& builder);
		KeyItemContext Key(std::string key) = delete;
		SingleValueItemContext Value(Node::Value value) = delete;
		DictItemContext StartDict() = delete;
		ArrayItemContext StartArray() = delete;
		Builder& EndDict() = delete;
	};

	class DictItemContext : public ItemContext {
	public:
		DictItemContext(Builder& builder);
		SingleValueItemContext Value(Node::Value value) = delete;
		DictItemContext StartDict() = delete;
		ArrayItemContext StartArray() = delete;
		Builder& EndArray() = delete;
		Node Build() = delete;
	};

	class ArrayItemContext : public ItemContext {
	public:
		ArrayItemContext(Builder& builder);
		ArrayItemContext Value(Node::Value value);
		KeyItemContext Key(std::string key) = delete;
		Builder& EndDict() = delete;
		Node Build() = delete;
	};
}
