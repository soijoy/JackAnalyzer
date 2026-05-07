#include <iostream>        // 1. 画面表示用
#include "JackTokenizer.h" // 2. Tokenizerを使いたいから設計図（目次）を読み込む

// 3. プログラムの始まり
int main(int argc, char* argv[]) {
    if (argc != 2) {       // 4. ファイル名が指定されているかチェック
        std::cout << "Usage: JackAnalyzer <filename.jack>" << std::endl;
        return 1;
    }

    // 5. 実体化！設計図(JackTokenizer)を元に、実際の道具(tokenizer)を1個作る
    //    ここで自動的に .cpp のコンストラクタが実行される
    JackTokenizer tokenizer(argv[1]);

    std::cout << "完了！" << std::endl;

    return 0;
