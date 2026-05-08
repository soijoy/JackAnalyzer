#include "CompilationEngine.h"

CompilationEngine::CompilationEngine(JackTokenizer* tokenizer, std::ofstream& outputFile)
    : tk(tokenizer), out(outputFile) {
    // 準備完了
}

void CompilationEngine::process() {
    TokenType type = tk->tokenType();
    std::string tag;

	// 種類に応じてタグを決定
    if (type == TokenType::KEYWORD) tag = "keyword";
	else if (type == TokenType::SYMBOL) tag = "symbol";
    else if (type == TokenType::IDENTIFIER) tag = "identifier";
    else if (type == TokenType::INT_CONST) tag = "integerConstant";
    else if (type == TokenType::STRING_CONST) tag = "stringConstant";
    else return; // NONEの場合は何もしない

	out << "<" << tag << "> ";
    
	// SYMBOLの場合は特殊文字をエスケープ
    if (type == TokenType::SYMBOL) {
		char s = tk->symbol();
        if (s == '<') out << "&lt;";
        else if (s == '>') out << "&gt;";
        else if (s == '&') out << "&amp;";
        else out << s;

    }
    else if (type == TokenType::STRING_CONST) {
        out << tk->stringVal();
    }
    else if (type == TokenType::INT_CONST) {
        out << tk->intVal();
    }
    else if (type == TokenType::KEYWORD) {
		// キーワードはそのまま出力
        out << tk->getCurrentToken();
    }
    else {
        out << tk->identifier();
    }

    out << "</" << tag << ">" << std::endl;

    if (tk->hasMoreTokens()) {
        tk->advance();
    }
}

void CompilationEngine::compileClass() {
    out << "<class>" << std::endl;

    process(); // classを処理
	process(); // class名を処理
	process(); // {を処理

	//classVarDec,Subroutineの処理

	process(); // }を処理
    out << "</class>" << std::endl;
}