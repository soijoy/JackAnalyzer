#include <iostream>
#include <fstream>
#include <filesystem>
#include "JackTokenizer.h"
#include "CompilationEngine.h"

namespace fs = std::filesystem;

void runTokenizer(std::string inputPath) {
    // 出力ファイル名の決定
    std::string outputPath = inputPath.substr(0, inputPath.find_last_of(".")) + ".vm";
    std::ofstream outFile(outputPath);

    JackTokenizer tokenizer(inputPath);
    CompilationEngine engine(&tokenizer, outFile);

    // クラスのコンパイルを開始
    engine.compileClass();
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
