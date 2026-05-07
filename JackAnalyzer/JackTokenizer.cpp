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

	content = std::regex_replace(content, std::regex("/\\*.*?\\*/", std::regex_constants::format_first_only | std::regex::extended), " ");
	content = std::regex_replace(content, std::regex("//.*"), " ");

    // 10. 記号を見つけたら前後にスペースを入れる
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

    // 11. スペースだらけになった文字列を、単語（トークン）のリストにする
    tokenize(spacedContent);
}

// 12. tokenize関数の「中身」
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