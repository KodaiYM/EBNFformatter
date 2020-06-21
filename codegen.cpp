#include "codegen.h"
#include <cassert>
#include <iostream>

void gen(const std::unique_ptr<Node> &node) {
	/* termination */
	if (Node::node_type::string == node->type) {
		assert(node->child.empty());
		std::cout << node->value;
		return;
	}

	if (Node::node_type::identifier == node->type) {
		assert(node->child.empty());
		std::cout << node->value;
		return;
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
		if (node->child.at(0)->child.size() >= 2) {
			std::cout << "(", gen(node->child.at(0)), std::cout << ")" << type;
		} else {
			gen(node->child.at(0)), std::cout << type;
		}

		return;
	}

	// binary operator
	if (Node::node_type::equal == node->type) {
		assert(node->child.size() == 2);
		assert(node->child.at(0)->type == Node::node_type::identifier);
		assert(node->child.at(1)->type == Node::node_type::either);

		gen(node->child.at(0)), std::cout << " ::= ";
		const auto &rhs = node->child.at(1);
		for (std::size_t i = 0; i < rhs->child.size(); ++i) {
			if (i != 0) {
				std::cout << " | ";
			}
			gen(rhs->child.at(i));
		}

		return;
	}

	/* variable arguments */
	if (Node::node_type::either == node->type) {
		assert(node->child.size() >= 1);

		for (size_t i = 0; i < node->child.size(); ++i) {
			if (i != 0) {
				std::cout << " | ";
			}
			gen(node->child.at(i));
		}

		return;
	}
	if (Node::node_type::series == node->type) {
		assert(node->child.size() >= 1);

		for (size_t i = 0; i < node->child.size(); ++i) {
			if (i != 0) {
				std::cout << " ";
			}
			if (node->child.at(i)->child.size() >= 2) {
				std::cout << "(", gen(node->child.at(i)), std::cout << ")";
			} else {
				gen(node->child.at(i));
			}
		}

		return;
	}

	// multiple lines
	if (Node::node_type::equals == node->type) {
		assert(node->child.size() >= 1);

		for (const auto &node_child : node->child) {
			gen(node_child), std::cout << std::endl;
		}

		return;
	}

	std::cerr << "not implemented type(" << static_cast<int>(node->type) << ")"
	          << std::endl;
	std::exit(EXIT_FAILURE);
}
