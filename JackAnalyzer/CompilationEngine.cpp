#include "CompilationEngine.h"

CompilationEngine::CompilationEngine(JackTokenizer* tokenizer, std::ofstream& outputFile)
    : tk(tokenizer), out(outputFile) {
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

    // クラス変数の宣言（static か field が続く限りループ）
    while (tk->getCurrentToken() == "static" || tk->getCurrentToken() == "field") {
        compileClassVarDec();
    }

    // サブルーチンの宣言（constructor, function, method が続く限りループ）
    while (tk->getCurrentToken() == "constructor" ||
        tk->getCurrentToken() == "function" ||
        tk->getCurrentToken() == "method") {
        compileSubroutine();
    }

	process(); // }を処理
    out << "</class>" << std::endl;
}

void CompilationEngine::compileClassVarDec() {
    out << "<classVarDec>" << std::endl;
    process(); // static or field
    process(); // type
    process(); // varName
    // 変数が複数ある場合の処理
    while (tk->getCurrentToken() == ",") {
        process(); // ,
        process(); // varName
    }
    process(); // ;
    out << "</classVarDec>" << std::endl;
}

void CompilationEngine::compileSubroutine() {
    out << "<subroutineDec>" << std::endl;
    process(); // constructor, function, method
    process(); // type
    process(); // subroutineName
    process(); // (
    compileParameterList();
    process(); // )
    out << "<subroutineBody>" << std::endl;
    process(); // {
    while (tk->getCurrentToken() == "var") {
        compileVarDec();
    }
    compileStatements();
    process(); // }
    out << "</subroutineBody>" << std::endl;
    out << "</subroutineDec>" << std::endl;
}

void CompilationEngine::compileParameterList() {
    out << "<parameterList>" << std::endl;

    // 次のトークンが ')' でないなら、引数がある
    if (tk->getCurrentToken() != ")") {
        process(); // 型
        process(); // 変数名

        // 2つ目以降の引数がある場合 (カンマが続く限り)
        while (tk->getCurrentToken() == ",") {
            process(); // ','
            process(); // 型
            process(); // 変数名
        }
    }

    out << "</parameterList>" << std::endl;
}

void CompilationEngine::compileVarDec() {
    out << "<varDec>" << std::endl;

    process(); // 'var'
    process(); // 型
    process(); // 変数名

    while (tk->getCurrentToken() == ",") {
        process(); // ','
        process(); // 変数名
    }

    process(); // ';'
    out << "</varDec>" << std::endl;
}

void CompilationEngine::compileStatements() {
    out << "<statements>" << std::endl;

    // 現在のトークンを見て、5つのキーワードのどれかである限り繰り返す
    while (true) {
        std::string t = tk->getCurrentToken();
        if (t == "let") compileLet();
        else if (t == "if") compileIf();
        else if (t == "while") compileWhile();
        else if (t == "do") compileDo();
        else if (t == "return") compileReturn();
        else break; // 5つ以外（つまり '}'）が来たら終了
    }

    out << "</statements>" << std::endl;
}

void CompilationEngine::compileLet() {
    out << "<letStatement>" << std::endl;

    process(); // 'let'
    process(); // 変数名

    // 配列の指定 [expression] があるかチェック
    if (tk->getCurrentToken() == "[") {
        process(); // '['
        compileExpression(); // 添字の解析
        process(); // ']'
    }

    process(); // '='
    compileExpression(); // 代入する値（式）の解析
    process(); // ';'

    out << "</letStatement>" << std::endl;
}

void CompilationEngine::compileWhile() {
    out << "<whileStatement>" << std::endl;

    process(); // 'while'
    process(); // '('
    compileExpression(); // 継続条件（式）を解析
    process(); // ')'

    process(); // '{'
    compileStatements(); // ループ内の中身（文）を再帰的に解析
    process(); // '}'

    out << "</whileStatement>" << std::endl;
}

void CompilationEngine::compileIf() {
    out << "<ifStatement>" << std::endl;

    process(); // 'if'
    process(); // '('
    compileExpression(); // 条件式を解析
    process(); // ')'

    process(); // '{'
    compileStatements(); // if内の中身を解析
    process(); // '}'

    // 次のトークンが 'else' なら、else節も解析する
    if (tk->getCurrentToken() == "else") {
        process(); // 'else'
        process(); // '{'
        compileStatements(); // else内の中身を解析
        process(); // '}'
    }

    out << "</ifStatement>" << std::endl;
}

void CompilationEngine::compileDo() {
    out << "<doStatement>" << std::endl;

    process(); // 'do'

    // サブルーチン呼び出しの解析
    // ここは少し複雑なので、まずは「;」が来るまでprocess()を回す形でも
    // 構造チェックは通りますが、本来は識別子やドットを正確に追います。
    process(); // クラス名 または 変数名 または 関数名

    if (tk->getCurrentToken() == ".") {
        process(); // '.'
        process(); // 関数名
    }

    process(); // '('
    compileExpressionList(); // 引数の解析
    process(); // ')'
    process(); // ';'

    out << "</doStatement>" << std::endl;
}

void CompilationEngine::compileReturn() {
    out << "<returnStatement>" << std::endl;

    process(); // 'return'

    // 次が ';' でないなら、戻り値の「式」がある
    if (tk->getCurrentToken() != ";") {
        compileExpression();
    }

    process(); // ';'

    out << "</returnStatement>" << std::endl;
}

void CompilationEngine::compileExpression() {
    out << "<expression>" << std::endl;

    // 最初の項を解析
    compileTerm();

    // 次に演算子 (+, -, *, /, &, |, <, >, =) が続く限り、項を追加していく
    std::string t = tk->getCurrentToken();
    while (t == "+" || t == "-" || t == "*" || t == "/" || t == "&" ||
        t == "|" || t == "<" || t == ">" || t == "=") {
        process();      // 演算子を出力
        compileTerm();  // 次の項を解析
        t = tk->getCurrentToken();
    }

    out << "</expression>" << std::endl;
}

void CompilationEngine::compileExpressionList() {
    out << "<expressionList>" << std::endl;

    // 次が ')' でないなら、少なくとも1つの式がある
    if (tk->getCurrentToken() != ")") {
        compileExpression();

        // 2つ目以降がある場合（カンマが続く限り）
        while (tk->getCurrentToken() == ",") {
            process(); // ','
            compileExpression();
        }
    }

    out << "</expressionList>" << std::endl;
}

void CompilationEngine::compileTerm() {
    out << "<term>" << std::endl;

    TokenType type = tk->tokenType();
    std::string t = tk->getCurrentToken();

    if (type == TokenType::INT_CONST || type == TokenType::STRING_CONST) {
        process(); // 数字や文字列はそのまま
    }
    else if (t == "true" || t == "false" || t == "null" || t == "this") {
        process(); // キーワード定数
    }
    else if (t == "(") {
        process(); // '('
        compileExpression(); // 括弧の中はまた「式」
        process(); // ')'
    }
    else if (t == "-" || t == "~") {
        process(); // 単項演算子 (-x や ~true)
        compileTerm(); // その後に続く「項」を解析
    }
    else if (type == TokenType::IDENTIFIER) {
        // ここがポイント！変数名か、配列か、関数呼び出し（. か (）かを先読み
        process(); // とりあえず名前（identifier）を処理

        std::string next = tk->getCurrentToken();
        if (next == "[") {
            process(); // '['
            compileExpression();
            process(); // ']'
        }
        else if (next == "(") {
            process(); // '('
            compileExpressionList();
            process(); // ')'
        }
        else if (next == ".") {
            process(); // '.'
            process(); // サブルーチン名
            process(); // '('
            compileExpressionList();
            process(); // ')'
        }
    }

    out << "</term>" << std::endl;
}