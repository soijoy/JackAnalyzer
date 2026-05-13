#include "SymbolTable.h"

SymbolTable::SymbolTable() {
    // コンストラクタでカウンターを初期化
    counts[Kind::STATIC] = 0;
    counts[Kind::FIELD] = 0;
    counts[Kind::ARG] = 0;
    counts[Kind::VAR] = 0;
}

void SymbolTable::startSubroutine() {
    // 新しい関数が始まったら、ローカルな辞書を空にする
    subroutineSymbols.clear();
    // カウンターもリセット（引数とローカル変数のみ）
    counts[Kind::ARG] = 0;
    counts[Kind::VAR] = 0;
}

void SymbolTable::define(std::string name, std::string type, Kind kind) {
    int index = counts[kind]++; // 現在のカウントを使い、その後+1する
    Symbol s = { type, kind, index };

    if (kind == Kind::STATIC || kind == Kind::FIELD) {
        classSymbols[name] = s;
    }
    else {
        subroutineSymbols[name] = s;
    }
}

int SymbolTable::indexOf(std::string name) {
    // まずサブルーチン（ローカル）を探す
    if (subroutineSymbols.find(name) != subroutineSymbols.end()) {
        return subroutineSymbols[name].index;
    }
    // なければクラス（グローバル）を探す
    if (classSymbols.find(name) != classSymbols.end()) {
        return classSymbols[name].index;
    }
    return -1; // 見つからない場合
}