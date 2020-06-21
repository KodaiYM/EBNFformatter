#include "parser.h"
#include "error.h"
#include <cassert>
#include <iostream>

using namespace std::string_literals;
namespace std {
/* basic_string + basic_string_view */
template <class charT, class traits, class Allocator>
basic_string<charT, traits, Allocator>
operator+(basic_string_view<charT, traits>              lhs,
          const basic_string<charT, traits, Allocator> &rhs) {
	return std::basic_string<charT, traits, Allocator>(lhs) + rhs;
}
template <class charT, class traits, class Allocator>
basic_string<charT, traits, Allocator>
operator+(basic_string_view<charT, traits>         lhs,
          basic_string<charT, traits, Allocator> &&rhs) {
	return std::basic_string<charT, traits, Allocator>(lhs) + std::move(rhs);
}
template <class charT, class traits, class Allocator>
basic_string<charT, traits, Allocator>
operator+(const basic_string<charT, traits, Allocator> &lhs,
          basic_string_view<charT, traits>              rhs) {
	return lhs + std::basic_string<charT, traits, Allocator>(rhs);
}
template <class charT, class traits, class Allocator>
basic_string<charT, traits, Allocator>
operator+(basic_string<charT, traits, Allocator> &&lhs,
          basic_string_view<charT, traits>         rhs) {
	return std::move(lhs) + std::basic_string<charT, traits, Allocator>(rhs);
}
} // namespace std

bool Parser::consume(std::string_view op) {
	if (token_list.empty()) {
		return false;
	}
	auto token = token_list.front();
	if (op == token.value) {
		token_list.pop_front();
		return true;
	} else {
		return false;
	}
}
void Parser::expect(std::string_view op) {
	const auto current_token = token_list.front();
	if (!consume(op)) {
		error("Token '"s + op + "' was expected, but not.", current_token.line,
		      current_token.line_num, current_token.pos);
	}
}
std::string Parser::expect_identifier() {
	if (token_list.empty()) {
		throw std::out_of_range("token queue is empty.");
	}

	const auto  current_token = token_list.front();
	const auto &token         = current_token.value;
	token_list.pop_front();
	if (token.find_first_not_of(
	        "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_0123456789") !=
	    token.npos) {
		error("An identifier token was expected, but not.", current_token.line,
		      current_token.line_num, current_token.pos);
	}

	return token;
}
std::string Parser::expect_string() {
	if (token_list.empty()) {
		throw std::out_of_range("token queue is empty.");
	}

	const auto  current_token = token_list.front();
	const auto &token         = current_token.value;
	token_list.pop_front();
	if ('"' != current_token.value.at(0)) {
		error("A string literal was expected, but not.", current_token.line,
		      current_token.line_num, current_token.pos);
	}

	return token;
}

std::unique_ptr<Node> Parser::new_node(Node::node_type type,
                                       std::string     value) {
	switch (type) {
	case Node::node_type::string:
	case Node::node_type::identifier: {
		std::unique_ptr<Node> node(new Node(type));
		node->value = value;
		return node;
	} break;
	default:
		/* non terminal type */
		throw std::invalid_argument("Node::node_type = "s +
		                            std::to_string(static_cast<int>(type)) +
		                            " is not a terminal node.");
		break;
	}
}

std::unique_ptr<Node> Parser::source() {
	std::unique_ptr<Node> node(new Node(Node::node_type::equals));

	while (!token_list.empty()) {
		node->child.push_back(equation());
		if (!consume("\n")) {
			break;
		}
	}

	return node;
}

std::unique_ptr<Node> Parser::equation() {
	auto node = new_node(Node::node_type::identifier, expect_identifier());

	if (!consume("::=")) {
		expect("=");
	}

	return new_node(Node::node_type::equal, std::move(node), either());
}
std::unique_ptr<Node> Parser::either() {
	auto node = new_node(Node::node_type::either, primary());

	while (consume("|")) {
		node->child.push_back(primary());
	}

	return node;
}
std::unique_ptr<Node> Parser::primary() {
	std::unique_ptr<Node> series_node(new Node(Node::node_type::series));

	while (1) {
		std::unique_ptr<Node> node;

		if (consume("(")) {
			node = either();
			expect(")");
		} else if ('"' == token_list.front().value.front()) {
			node = new_node(Node::node_type::string, expect_string());
		} else if (
		    "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_0123456789"s
		        .find_first_of(token_list.front().value.front()) !=
		    std::string::npos) {
			node = new_node(Node::node_type::identifier, expect_identifier());
		} else {
			break;
		}

		if (consume("?")) {
			series_node->child.push_back(
			    new_node(Node::node_type::question, std::move(node)));
		} else if (consume("*")) {
			series_node->child.push_back(
			    new_node(Node::node_type::star, std::move(node)));
		} else if (consume("+")) {
			series_node->child.push_back(
			    new_node(Node::node_type::plus, std::move(node)));
		} else {
			series_node->child.push_back(std::move(node));
		}
	}

	return series_node;
}

std::unique_ptr<Node> Parser::makeAST() {
	auto AST = source();
	if (!token_list.empty()) {
		const auto &extra_token = token_list.front();
		error("extra character", extra_token.line, extra_token.line_num,
		      extra_token.pos);
	}

	return AST;
}
