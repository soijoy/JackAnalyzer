#pragma once
#include <string>
#include <unordered_map>

// 変数の属性（種類）
enum class Kind { STATIC, FIELD, ARG, VAR, NONE };

// 変数1つ分のデータ
struct Symbol {
    std::string type; // int, char, boolean, またはクラス名
    Kind kind;        // STATIC, FIELD, ARG, VAR
    int index;        // 0, 1, 2...（種類ごとにカウント）
};


class SymbolTable {
public:
    SymbolTable();

    // 新しいサブルーチンスコープを開始する（関数の解析が始まる時に呼ぶ）
    void startSubroutine();

    // 変数を登録する（例: name="x", type="int", kind=VAR）
    void define(std::string name, std::string type, Kind kind);

    // 引数やローカル変数など、指定された属性の変数が今いくつあるか返す
    int varCount(Kind kind);

    // 変数名からその属性を返す
    Kind kindOf(std::string name);

    // 変数名からその型を返す
    std::string typeOf(std::string name);

    // 変数名からそのインデックス（番号）を返す
    int indexOf(std::string name);

private:
    // クラスレベルの辞書 (static, field)
    std::unordered_map<std::string, Symbol> classSymbols;
    // サブルーチンレベルの辞書 (argument, local)
    std::unordered_map<std::string, Symbol> subroutineSymbols;

    // 各属性のインデックスを管理するカウンター
    std::unordered_map<Kind, int> counts;
};

