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
		std::vector<std::string> itemKeys_;
		std::stack<Node*> queue_;
		std::size_t dictcount = 0;
		bool objectReady = false;
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

	class KeyItemContext {
	private:
		Builder& builder_;
	public:
		KeyItemContext(Builder& builder);
		DictItemContext Value(Node::Value value);
		DictItemContext StartDict();
		ArrayItemContext StartArray();
	};

	class SingleValueItemContext {
	private:
		Builder& builder_;
	public:
		SingleValueItemContext(Builder& builder);
		Builder& EndArray();
		Node Build() {
			return builder_.Build();
		}
	};

	class DictItemContext {
	private:
		Builder& builder_;
	public:
		DictItemContext(Builder& builder);
		KeyItemContext Key(std::string key);
		Builder& EndDict();
	};

	class ArrayItemContext {
	private:
		Builder& builder_;
	public:
		ArrayItemContext(Builder& builder);
		ArrayItemContext Value(Node::Value value);
		DictItemContext StartDict();
		ArrayItemContext StartArray();
		Builder& EndArray();
	};
}
