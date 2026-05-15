#pragma once
#include <string>
#include <vector>

enum class TokenType {
	KEYWORD, SYMBOL, IDENTIFIER, INT_CONST, STRING_CONST, NONE
};

class JackTokenizer {
public:
	JackTokenizer(std::string filename);

	bool hasMoreTokens();
	void advance();
	TokenType tokenType(); // 現在のトークンの種類を返す
	std::string keyWord();
	char symbol();
	std::string identifier();
	int intVal();
	std::string stringVal();

	// Provide read-only access to tokens so external code doesn't need to access the
	// private member directly.
	const std::vector<std::string>& getTokens() const { return tokens; }

	std::string getCurrentToken() const;
		
private:
	std::vector<std::string> tokens;
	size_t currentPos = 0;
	const std::string symbols = "{}()[].,;+-*/&|<>=~";

	void tokenize(const std::string& input);
};