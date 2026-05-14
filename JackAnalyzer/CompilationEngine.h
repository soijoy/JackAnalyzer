#pragma once
#include <fstream>
#include <string>
#include "JackTokenizer.h"
#include "SymbolTable.h"
#include "VMWriter.h"

class CompilationEngine {
public:
    // コンストラクタ：入力のTokenizerと出力先のファイルをセットする
    CompilationEngine(JackTokenizer* tokenizer, std::ofstream& outputFile);

    // 文法規則ごとのコンパイルメソッド群
    void compileClass();
    void compileClassVarDec();
    void compileSubroutine();
    void compileParameterList();
    void compileVarDec();
    void compileStatements();
    void compileLet();
    void compileIf();
    void compileWhile();
    void compileDo();
    void compileReturn();
    void compileExpression();
    void compileTerm();
    int compileExpressionList();
    
    std::string getCurrentToken() const;

private:
    JackTokenizer* tk;
    std::ofstream& out;
    SymbolTable symbolTable;
    VMWriter vw;

    // XML出力のためのヘルパー関数（あると便利！）
    void process(); // 現在のトークンをXML形式で書き出す
    std::string kindToSegment(Kind kind);
};