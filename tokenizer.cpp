#include "tokenizer.h"
#include "error.h"
#include <vector>

using namespace std::string_literals;

void Tokenizer::remove_prefix(std::string_view &str, std::size_t count) {
	str.remove_prefix(count);
	remove_length += count;
}
Tokenizer::Tokenizer(std::string_view token_str)
    : token_str(token_str) {
	std::size_t line_num = 0; // token line index
	while (token_str.length()) {
		/* consume to a CR or LF */
		std::string_view token_line;
		{
			auto first_LF_index = token_str.find_first_of("\n");
			if (token_str.npos == first_LF_index) {
				token_line = token_str;
				token_str.remove_prefix(token_str.size());
			} else {
				auto first_CR_index = token_str.find_first_of("\r");
				// CRLF
				if (first_CR_index == first_LF_index - 1) {
					token_line = token_str.substr(0, first_CR_index);
					token_str.remove_prefix(first_LF_index + 1);
				} else {
					token_line = token_str.substr(0, first_LF_index);
					token_str.remove_prefix(first_LF_index + 1);
				}
			}
		}

		// initialize remove_length
		remove_length = 0;

		/* consume a line token */
		std::string this_line(token_line);
		while (token_line.length()) {
			/* remove blanks */
			if (std::isblank(token_line.front())) {
				auto not_space_first_it =
				    std::find_if(token_line.begin(), token_line.end(),
				                 [](char c) { return std::isblank(c) == 0; });
				remove_prefix(token_line,
				              std::distance(token_line.begin(), not_space_first_it));

				/* finish */
				if (!token_line.length()) {
					break;
				}
			}

			/* string literal */
			if (auto &front = token_line.front(); '"' == front) {
				remove_prefix(token_line, 1);
				auto doublequote_index = token_line.find('"');
				if (token_line.npos == doublequote_index) {
					error("expect \"", token_line, line_num, token_line.size());
				}

				auto string_literal = token_line.substr(0, doublequote_index);
				token_list.push_back(Token{'"' + std::string(string_literal) + '"',
				                           this_line, line_num, remove_length - 1});
				remove_prefix(token_line, doublequote_index + 1);
				continue;
			}

			/* symbol */
			{
				const std::vector<const char *> operators = {"::=", "=", "(", ")",
				                                             "+",   "*", "?", "|"};
				if (auto found_op = std::find_if(operators.begin(), operators.end(),
				                                 [&token_line](const auto &op) {
					                                 return token_line.starts_with(op);
				                                 });
				    found_op != operators.end()) {
					std::string op(*found_op);
					token_list.push_back(Token{op, this_line, line_num, remove_length});
					remove_prefix(token_line, op.length());
					continue;
				}
			}

			/* identifier */
			if (auto &front = token_line.front(); isalpha(front)) {
				auto first_not_identifier_index = token_line.find_first_not_of(
				    "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_");
				if (token_line.npos == first_not_identifier_index) {
					first_not_identifier_index = token_line.length();
				}

				auto identifier = token_line.substr(0, first_not_identifier_index);
				token_list.push_back(
				    Token{std::string(identifier), this_line, line_num, remove_length});
				remove_prefix(token_line, first_not_identifier_index);
				continue;
			}

			error("Invalid token: "s + token_line.front(), this_line, line_num,
			      remove_length);
		}

		// add LF token
		token_list.push_back(Token{"\n", this_line, line_num, this_line.size()});

		// increment line num
		++line_num;
	}
}
