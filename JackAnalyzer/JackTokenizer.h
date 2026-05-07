#pragma once
#include <string>
#include <vector>

class JackTokenizer {
public:
	JackTokenizer(std::string filename);

	bool hasMoreTokens();
	void advance();
	std::string tokenType();
	std::string keyWord();
	std::string symbol();
	std::string identifier();
	int intVal();
	std::string stringVal();

private:
	std::vector<std::string> tokens;
	size_t currentPos = 0;
	const std::string symbols = "{}()[].,;+-*/&|<>=~";

	void tokenize(const std::string& input);
};