#include <iostream>
#include <fstream>
#include <filesystem>
#include "JackTokenizer.h"
#include "CompilationEngine.h"

namespace fs = std::filesystem;

void runTokenizer(std::string inputPath) {
    // 1. 出力ファイル名の決定 (Main.jack -> MainT.xml)
    std::string outputPath = inputPath.substr(0, inputPath.find_last_of(".")) + ".xml";
    std::ofstream outFile(outputPath);

    JackTokenizer tokenizer(inputPath);
	CompilationEngine engine(&tokenizer, outFile);
	engine.compileClass(); // クラスのコンパイルを開始


    outFile << "<tokens>" << std::endl;
    while (tokenizer.hasMoreTokens()) {
        TokenType type = tokenizer.tokenType();
        if (type == TokenType::KEYWORD) {
            outFile << "<keyword> " << (std::string)tokenizer.keyWord() << " </keyword>" << std::endl;
        }
        else if (type == TokenType::SYMBOL) {
            outFile << "<symbol> " << (char)tokenizer.symbol() << " </symbol>" << std::endl;
        }
        else if (type == TokenType::IDENTIFIER) {
            outFile << "<identifier> " << (std::string)tokenizer.identifier() << " </identifier>" << std::endl;
        }
        else if (type == TokenType::INT_CONST) {
            outFile << "<integerConstant> " << (int)tokenizer.intVal() << " </integerConstant>" << std::endl;
        }
        else if (type == TokenType::STRING_CONST) {
            outFile << "<stringConstant> " << (std::string)tokenizer.stringVal() << " </stringConstant>" << std::endl;
        }
        else {}
        tokenizer.advance();
    }
    outFile << "</tokens>" << std::endl;
}


int main(int argc, char* argv[]) {
    if (argc != 2) {       // ファイル名が指定されているかチェック
        std::cout << "Usage: JackAnalyzer <filename.jack>" << std::endl;
        return 1;
    }

    std::string path = argv[1];

    if (fs::is_directory(path)) {
        // フォルダ内の全ファイルをループ
        for (const auto& entry : fs::directory_iterator(path)) {
            if (entry.path().extension() == ".jack") {
                runTokenizer(entry.path().string());
            }
        }
    }
    else {
        // 単一ファイルの場合
        runTokenizer(path);
    }

    return 0;
}
