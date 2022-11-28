#include "headers/json_builder.h"
#include <iostream>
#include <exception>
#include <utility>
#include <type_traits>

namespace json {

	KeyItemContext Builder::Key(std::string key) {
		if (queue_.empty() || !queue_.top()->IsMap()) {
			throw std::logic_error("Dictonary not found, or value expected");
		}
		queue_.push(new Node(std::move(key)));
		return { *this };
	}

	SingleValueItemContext Builder::Value(Node::Value value) {
		if (queue_.empty() && root_.GetValue().index() != 0) {
			throw std::logic_error("Object already ready");
		}

		if (queue_.empty()) {
			root_.Swap(std::move(value));
		}
		else {
			Node* last = queue_.top();
			if (last->IsArray()) {
				last->AddValue(std::move(value));
			}
			else {
				std::string key = last->AsString();
				queue_.pop();
				queue_.top()->AddValue(last->AsString(), std::move(value));
			}
		}
		return { *this };
	}

	DictItemContext Builder::StartDict() {
		if (!queue_.empty() && queue_.top()->IsMap()) {
			throw std::logic_error("Dictonary key expected");
		}
		queue_.push(new Node(Dict{}));
		return { *this };
	}

	Builder& Builder::EndDict() {
		if (queue_.empty() || !queue_.top()->IsMap()) {
			throw std::logic_error("Array not found");
		}

		Node top = std::move(*(queue_.top()));
		queue_.pop();

		if (queue_.empty()) {
			root_ = std::move(top);
		}
		else {
			Node* parent = queue_.top();
			if (parent->IsArray()) {
				parent->AddValue(top.AsMap());
			}
			else {
				std::string key = parent->AsString();
				queue_.pop();
				queue_.top()->AddValue(key, top.AsMap());
			}
		}

		return *this;
	}

	ArrayItemContext Builder::StartArray() {
		queue_.push(new Node(Array{}));
		return { *this };
	}

	Builder& Builder::EndArray() {
		if (queue_.empty() || !queue_.top()->IsArray()) {
			throw std::logic_error("Array not found");
		}

		Node top = std::move(*(queue_.top()));
		queue_.pop();
		if (queue_.empty()) {
			root_ = std::move(top);
		}
		else {
			Node* parent = queue_.top();
			if (parent->IsArray()) {
				parent->AddValue(top.AsArray());
			}
			else {
				std::string key = parent->AsString();
				queue_.pop();
				queue_.top()->AddValue(key, top.AsArray());
			}
		}
		return *this;
	}

	Node Builder::Build() {
		if (!queue_.empty() || root_.GetValue().index() == 0) {
			throw std::logic_error("Object not ready");
		}
		return root_;
	}

	ItemContext::ItemContext(Builder& builder) : builder_(builder) {}
	KeyItemContext ItemContext::Key(std::string key) {
		return builder_.Key(key);
	}

	SingleValueItemContext ItemContext::Value(Node::Value value) {
		return builder_.Value(value);
	}

	DictItemContext ItemContext::StartDict() {
		return builder_.StartDict();
	}

	ArrayItemContext ItemContext::StartArray() {
		return builder_.StartArray();
	}

	Builder& ItemContext::EndDict() {
		return builder_.EndDict();
	}

	Builder& ItemContext::EndArray() {
		return builder_.EndArray();
	}

	Node ItemContext::Build() {
		return builder_.Build();
	}

	KeyItemContext::KeyItemContext(Builder& builder) : ItemContext(builder) {}
	SingleValueItemContext::SingleValueItemContext(Builder& builder) : ItemContext(builder) {}
	DictItemContext::DictItemContext(Builder& builder) : ItemContext(builder) {}
	ArrayItemContext::ArrayItemContext(Builder& builder) : ItemContext(builder) {}

	DictItemContext KeyItemContext::Value(Node::Value value) {
		ItemContext::Value(value);
		return { builder_ };
	}

	ArrayItemContext ArrayItemContext::Value(Node::Value value) {
		ItemContext::Value(value);
		return { builder_ };
	}
}