#include "JackTokenizer.h"
#include <fstream>
#include <regex>
#include <iterator>

// コンストラクタの実装
JackTokenizer::JackTokenizer(std::string filename) {
	std::ifstream file(filename);
	if (!file.is_open()) {
		throw std::runtime_error("Could not open file: " + filename);
	}

	std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    // // コメントを消す
    size_t pos;
    while ((pos = content.find("//")) != std::string::npos) {
        size_t endOfLine = content.find("\n", pos);
        if (endOfLine == std::string::npos) endOfLine = content.length();
        content.erase(pos, endOfLine - pos);
    }

    // /* */ コメントを消す
    while ((pos = content.find("/*")) != std::string::npos) {
        size_t endPos = content.find("*/", pos);
        if (endPos != std::string::npos) {
            content.erase(pos, (endPos + 2) - pos);
        }
        else {
            break; // 閉じ忘れ対応
        }
    }

    std::string spacedContent = "";
    for (char c : content) {        // 文字を一文字ずつ見ていく
        if (symbols.find(c) != std::string::npos) {
            spacedContent += " ";   // 前にスペース 演算子オーバーロード
            spacedContent += c;     // 記号本体
            spacedContent += " ";   // 後ろにスペース
        }
        else {
            spacedContent += c;     // 記号じゃなければそのまま
        }
    }
    // スペースだらけになった文字列を、単語（トークン）のリストにする
    tokenize(spacedContent);
}



// tokenize関数
void JackTokenizer::tokenize(const std::string& input) {
    std::string current = "";
    bool inString = false;

    for (size_t i = 0; i < input.length(); ++i) {
        char c = input[i];

        if (c == '"') { // 文字列定数の開始または終了
            if (inString) {
                // 文字列定数の終わり：" を含めてトークンとして保存
                current += c;
                tokens.push_back(current);
                current = "";
                inString = false;
            }
            else {
                // 文字列定数の始まり
                inString = true;
                current += c;
            }
        }
        else if (inString) {
            // 文字列の中ならスペースもそのまま取り込む
            current += c;
        }
        else if (isspace(c)) {
            // 空白で区切る（文字列の外のみ）
            if (!current.empty()) {
                tokens.push_back(current);
                current = "";
            }
        }
        else {
            current += c;
        }
    }
    if (!current.empty()) tokens.push_back(current);
}

TokenType JackTokenizer::tokenType() {
	// 現在のトークンを取得
    const std::string& t = tokens[currentPos];
    
    // KEYWORDの判定
    if (t == "class" || t == "constructor" || t == "function" || t == "method" ||
        t == "field" || t == "static" || t == "var" || t == "int" ||
        t == "char" || t == "boolean" || t == "void" || t == "true" ||
        t == "false" || t == "null" || t == "this" || t == "let" ||
        t == "do" || t == "if" || t == "else" || t == "while" ||
        t == "return") {
        return TokenType::KEYWORD;
    }

	// SYMBOLの判定
    if (t.length() == 1 && symbols.find(t[0]) != std::string::npos) {
		return TokenType::SYMBOL;
    }

    // INT_CONSTの判定
    if (isdigit(t[0])) {
        return TokenType::INT_CONST;
    }
	// STRING_CONSTの判定
    if (t[0] == '"') {
		return TokenType::STRING_CONST;
    }

    // Otherwise IDENTIFIER
    return TokenType::IDENTIFIER;
}

// Return the keyword of the current token (valid when tokenType() == KEYWORD)
std::string JackTokenizer::keyWord() {
    return tokens[currentPos];
}

void JackTokenizer::advance() {
    if (hasMoreTokens()) {
        currentPos++;
    }
}

bool JackTokenizer::hasMoreTokens() {
    // 現在の位置が、トークン全体の数より小さければ「まだある」
    return currentPos < tokens.size();
}

// 記号を返す (TokenTypeがSYMBOLのときだけ呼ばれる想定)
char JackTokenizer::symbol() {
    return tokens[currentPos][0];
}

// 識別子（変数名など）を返す
std::string JackTokenizer::identifier() {
    return tokens[currentPos];
}

// 整数値を返す
int JackTokenizer::intVal() {
    return std::stoi(tokens[currentPos]); // 文字列を数値に変換する便利な関数
}

// 文字列定数を返す（前後の " を取り除いて返すのが仕様）
std::string JackTokenizer::stringVal() {
    const std::string& t = tokens[currentPos];
    // 最初と最後の " を除いた部分を抜き出す
    return t.substr(1, t.length() - 2);
}

std::string JackTokenizer::getCurrentToken() const {
    return tokens[currentPos];
}