#include "print.h"
#include <cassert>
#include <iostream>

std::ostream &operator<<(std::ostream &stream, const Tokenizer &tokenizer) {
	return stream << tokenizer.token_list;
}
std::ostream &operator<<(std::ostream &stream, std::list<Token> token_list) {
	stream << "{";

	stream << std::move(token_list.front().value);
	token_list.pop_front();
	while (!token_list.empty()) {
		stream << ", " << std::move(token_list.front().value);
		token_list.pop_front();
	}

	stream << "}";

	return stream;
}
std::ostream &operator<<(std::ostream &               stream,
                         const std::unique_ptr<Node> &node) {
	/* termination pattern */
	if (Node::node_type::identifier == node->type) {
		assert(node->child.empty());
		stream << node->value;
		return stream;
	}
	if (Node::node_type::string == node->type) {
		assert(node->child.empty());
		stream << node->value;
		return stream;
	}

	// unary operator
	if (Node::node_type::question == node->type ||
	    Node::node_type::star == node->type ||
	    Node::node_type::plus == node->type) {
		std::string type;

		switch (node->type) {
		case Node::node_type::question:
			type = '?';
			break;
		case Node::node_type::star:
			type = '*';
			break;
		case Node::node_type::plus:
			type = '+';
			break;
		default:
			assert(false);
			break;
		}

		assert(node->child.size() == 1);
		stream << "(" << node->child.at(0) << ")" << type;

		return stream;
	}

	// binary operator
	if (Node::node_type::equal == node->type) {
		assert(node->child.size() == 2);
		assert(node->child.at(0)->type == Node::node_type::identifier);
		assert(node->child.at(1)->type == Node::node_type::either);

		stream << node->child.at(0) << " ::= " << node->child.at(1);
		return stream;
	}

	// variable arguments
	if (Node::node_type::either == node->type ||
	    Node::node_type::series == node->type) {
		assert(node->child.size() >= 1);

		const auto separator = Node::node_type::either == node->type ? " | " : " ";

		if (Node::node_type::either == node->type) {
			stream << "(";
		}
		stream << node->child.at(0);
		for (size_t i = 1; i < node->child.size(); ++i) {
			stream << separator << node->child.at(i);
		}
		if (Node::node_type::either == node->type) {
			stream << ")";
		}

		return stream;
	}

	// multiple lines
	if (Node::node_type::equals == node->type) {
		assert(node->child.size() >= 1);

		for (const auto &node_child : node->child) {
			stream << node_child << std::endl;
		}

		return stream;
	}

	std::cerr << "Invalid type(" << static_cast<int>(node->type) << ") detected."
	          << std::endl;
	std::exit(EXIT_FAILURE);
}
