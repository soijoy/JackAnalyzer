#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <regex>

class JackTokenizer {
    std::vector<std::string> tokens;
    size_t currentPos = 0;

    // 記号の定義
    const std::string symbols = "{}()[].,;+-*/&|<>=~";

public:
    JackTokenizer(std::string filename) {
        std::ifstream file(filename);
        std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

        // 1. コメントを除去 (正規表現を使うと楽です)
        // /* ... */ (複数行) を除去
        content = std::regex_replace(content, std::regex("/\\*.*?\\*/", std::regex_constants::format_first_only | std::regex::extended), " ");
        // // ... (単一行) を除去
        content = std::regex_replace(content, std::regex("//.*"), " ");

        // 2. 記号の周りにスペースを入れる
        std::string spacedContent = "";
        for (char c : content) {
            if (symbols.find(c) != std::string::npos) {
                spacedContent += " ";
                spacedContent += c;
                spacedContent += " ";
            }
            else {
                spacedContent += c;
            }
        }

        // 3. スペースで区切って vector に格納
        // ※ただし、"String Constant" の中のスペースは区切ってはいけない！
        // ここは少し工夫が必要（一旦文字列定数を保護するか、自力で回すか）
        tokenize(spacedContent);
    }

private:
    void tokenize(const std::string& input) {
        // ここで文字列定数（"..."）を考慮しつつ分割するロジックを書く
        // 簡単な方法は、" が出てきたら次の " までを一気に取り出す処理です
    }
};

