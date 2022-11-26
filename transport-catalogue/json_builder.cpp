#include "headers/json_builder.h"
#include <iostream>
#include <exception>
#include <utility>
#include <variant>

namespace json {

	KeyItemContext Builder::Key(std::string key) {
		if (queue_.empty() || !queue_.top()->IsMap() || itemKeys_.size() >= dictcount) {
			throw std::logic_error("Dictonary not found, or value expected");
		}

		itemKeys_.push_back(std::move(key));
		//return *this;
		return { *this };
	}

	SingleValueItemContext Builder::Value(Node::Value value) {
		if (objectReady) {
			throw std::logic_error("Object already ready");
		}

		if (queue_.empty()) {
			root_.Swap(std::move(value));
			objectReady = true;
		}
		else {
			if (itemKeys_.size() != dictcount) {
				throw std::logic_error("Dictonary key not found");
			}
			Node* last = queue_.top();
			if (last->IsMap()) {
				last->AddValue(itemKeys_.back(), std::move(value));
				itemKeys_.pop_back();
			}
			else if (last->IsArray()) {
				last->AddValue(std::move(value));
			}
		}
		return { *this };
		//return *this;
	}

	DictItemContext Builder::StartDict() {
		if (!queue_.empty() && queue_.top()->IsMap() && itemKeys_.empty()) {
			throw std::logic_error("Dictonary key expected");
		}
		queue_.push(new Node(Dict{}));
		++dictcount;
		//return *this;
		return { *this };
	}

	Builder& Builder::EndDict() {
		if (queue_.empty() || !queue_.top()->IsMap() || itemKeys_.size() >= dictcount) {
			throw std::logic_error("Dictonary not found");
		}
		--dictcount;
		Node top = std::move(*(queue_.top()));
		queue_.pop();
		if (queue_.empty()) {
			root_ = std::move(top);
			objectReady = true;
		}
		else {
			Node* parent = queue_.top();
			if (parent->IsMap()) {
				parent->AddValue(itemKeys_.back(), top.AsMap());
				itemKeys_.pop_back();
			}

			if (parent->IsArray()) {
				parent->AddValue(top.AsMap());
			}
		}

		return *this;
	}

	ArrayItemContext Builder::StartArray() {
		if (objectReady) {
			throw std::logic_error("Object already ready");
		}

		if (itemKeys_.size() != dictcount) {
			throw std::logic_error("Dictonary key expected");
		}
		queue_.push(new Node(Array{}));
		//return *this;
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
			objectReady = true;
		}
		else {
			Node* parent = queue_.top();
			if (parent->IsMap()) {
				parent->AddValue(itemKeys_.back(), top.AsArray());
				itemKeys_.pop_back();
			}

			if (parent->IsArray()) {
				parent->AddValue(top.AsArray());
			}
		}
		return *this;
	}

	Node Builder::Build() {
		if (!objectReady || !queue_.empty()) {
			throw std::logic_error("Object not ready");
		}
		return root_;
	}

	KeyItemContext::KeyItemContext(Builder& builder) :builder_(builder) {}
	DictItemContext KeyItemContext::Value(Node::Value value) {
		builder_.Value(value);
		return { builder_ };
	}

	DictItemContext KeyItemContext::StartDict() {
		return builder_.StartDict();
	}

	ArrayItemContext KeyItemContext::StartArray() {
		return builder_.StartArray();
	}

	SingleValueItemContext::SingleValueItemContext(Builder& builder) :builder_(builder) {}
	Builder& SingleValueItemContext::EndArray() {
		return builder_.EndArray();
	}

	DictItemContext::DictItemContext(Builder& builder) :builder_(builder) {}
	KeyItemContext DictItemContext::Key(std::string key) {
		return builder_.Key(key);
	}

	Builder& DictItemContext::EndDict() {
		return builder_.EndDict();
	}

	ArrayItemContext::ArrayItemContext(Builder& builder) :builder_(builder) {}
	ArrayItemContext ArrayItemContext::Value(Node::Value value) {
		builder_.Value(value);
		return { builder_ };
	}

	DictItemContext ArrayItemContext::StartDict() {
		return builder_.StartDict();
	}

	ArrayItemContext ArrayItemContext::StartArray() {
		return builder_.StartArray();
	}

	Builder& ArrayItemContext::EndArray() {
		return builder_.EndArray();
	}
}