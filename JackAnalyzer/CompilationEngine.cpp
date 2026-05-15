#include "CompilationEngine.h"

CompilationEngine::CompilationEngine(JackTokenizer* tokenizer, std::ofstream& outputFile)
    : tk(tokenizer), out(outputFile), vw(outputFile) {
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

    if (tk->hasMoreTokens()) {
        tk->advance();
    }
}

void CompilationEngine::compileClass() {
    process(); // classを処理
    className = tk->getCurrentToken();
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
}

void CompilationEngine::compileClassVarDec() {
    std::string kindStr = tk->getCurrentToken();
	Kind kind = (kindStr == "static") ? Kind::STATIC : Kind::FIELD;
    process(); // static or field

	std::string type = tk->getCurrentToken();
    process(); // type

	std::string name = tk->getCurrentToken();
    symbolTable.define(name, type, kind);
    process(); // varName

    // 変数が複数ある場合の処理
    while (tk->getCurrentToken() == ",") {
        process(); // ,
		name = tk->getCurrentToken();
        symbolTable.define(name, type, kind);
        process(); // varName
    }
    process(); // ;
}

void CompilationEngine::compileSubroutine() {
	symbolTable.startSubroutine(); // 新しいサブルーチンスコープを開始

	std::string subroutineType = tk->getCurrentToken(); // constructor, function, method
    process(); // constructor, function, method

    process(); // type
	std::string subroutineName = tk->getCurrentToken();
    process(); // subroutineName

    process(); // (
    if (subroutineType == "method") {
        // thisを登録
        symbolTable.define("this", className, Kind::ARG);
    }
    compileParameterList();
    process(); // )
    process(); // {

    while (tk->getCurrentToken() == "var") {
        compileVarDec();
    }
    // ここでローカル変数の数が確定するので、function命令を出す
    int nLocals = symbolTable.varCount(Kind::VAR);
    vw.writeFunction(className + "." + subroutineName, nLocals);

    // サブルーチンの種類に応じた初期化（プロローグ）
    if (subroutineType == "constructor") {
        // 1. クラス変数の数だけメモリを確保
        int nFields = symbolTable.varCount(Kind::FIELD);
        vw.writePush("constant", nFields);
        vw.writeCall("Memory.alloc", 1);
        // 2. 確保したメモリのアドレスを 'this' (pointer 0) にセット
        vw.writePop("pointer", 0);
    }
    else if (subroutineType == "method") {
        // メソッドは、渡された第1引数(this)を pointer 0 にセットして開始する
        vw.writePush("argument", 0);
        vw.writePop("pointer", 0);
    }

    // 本体の文を実行
    compileStatements();
    process(); // '}'
}

void CompilationEngine::compileParameterList() {
    // 次のトークンが ')' でないなら、引数がある
    if (tk->getCurrentToken() != ")") {
        // first parameter: type then name
        std::string type = tk->getCurrentToken();
        process(); // consume type

        std::string name = tk->getCurrentToken();
        symbolTable.define(name, type, Kind::ARG); // register as ARG
        process(); // consume var name

        // subsequent parameters
        while (tk->getCurrentToken() == ",") {
            process(); // consume ','
            type = tk->getCurrentToken();
            process(); // consume type
            name = tk->getCurrentToken();
            symbolTable.define(name, type, Kind::ARG);
            process(); // consume var name
        }
    }
}

void CompilationEngine::compileVarDec() {
	std::string kindStr = tk->getCurrentToken();
	Kind kind = (kindStr == "var") ? Kind::VAR : Kind::NONE; // var以外はNONE
    process(); // 'var'

	std::string type = tk->getCurrentToken();
    process(); // 型

	std::string name = tk->getCurrentToken();
	symbolTable.define(name, type, kind);
    process(); // 変数名

    while (tk->getCurrentToken() == ",") {
        process(); // ','
        name = tk->getCurrentToken();
        symbolTable.define(name, type, kind);
        process(); // 変数名
    }

    process(); // ';'
}

void CompilationEngine::compileStatements() {
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
}

std::string CompilationEngine::kindToSegment(Kind kind) {
    switch (kind) {
        case Kind::STATIC:  return "static";
		case Kind::FIELD:   return "this";
		case Kind::ARG:     return "argument";
		case Kind::VAR:     return "local";
		default:            return ""; // NONEの場合は空文字
    }
}

void CompilationEngine::compileLet() {

    process(); // 'let'
    std::string varName = tk->getCurrentToken();
    process(); // 変数名

    bool isArray = false;
    if (tk->getCurrentToken() == "[") {
        isArray = true;
        process();
        compileExpression();
        process();

        // 左辺の配列のアドレスをプッシュ
        Kind kind = symbolTable.kindOf(varName);
        int index = symbolTable.indexOf(varName);
        vw.writePush(kindToSegment(kind), index);
		vw.writeArithmetic("add"); // ベースアドレス + インデックス
    }

	process(); // '='
	compileExpression(); // 右辺の式を解析
	process(); // ';'

    if (!isArray) {
        Kind kind = symbolTable.kindOf(varName);
        int index = symbolTable.indexOf(varName);
        vw.writePop(kindToSegment(kind), index);
    }
    else {
        // 配列への代入
        vw.writePop("temp", 0); // 右辺の値をスタックに残す
		vw.writePop("pointer", 1); // THATをターゲットアドレスにセット
		vw.writePush("temp", 0); // 右辺の値を再度スタックに戻す
		vw.writePop("that", 0); // THAT 0(a[i]) に値を格納
    }
}

void CompilationEngine::compileWhile() {
    std::string l1 = "WHILE_EXP" + std::to_string(labelIndex);
    std::string l2 = "WHILE END" + std::to_string(labelIndex);
    labelIndex++;
    vw.writeLabel(l1);

    process(); // 'while'
    process(); // '('
    compileExpression(); // 継続条件（式）を解析
    process(); // ')'

	vw.writeArithmetic("not");
	vw.writeIf(l2);

    process(); // '{'
    compileStatements(); // ループ内の中身（文）を再帰的に解析
    process(); // '}'

    vw.writeGoto(l1);
    vw.writeLabel(l2);
}

void CompilationEngine::compileIf() {
	std::string l1 = "IF_FALSE" + std::to_string(labelIndex);
	std::string l2 = "IF_END" + std::to_string(labelIndex);
	labelIndex++;

    process(); // 'if'
    process(); // '('
    compileExpression(); // 条件式を解析
    process(); // ')'

	vw.writeArithmetic("not");
	vw.writeIf(l1);

    process(); // '{'
    compileStatements(); // if内の中身を解析
    process(); // '}'

    vw.writeGoto(l2);
    vw.writeLabel(l1);

    // 次のトークンが 'else' なら、else節も解析する
    if (tk->getCurrentToken() == "else") {
        process(); // 'else'
        process(); // '{'
        compileStatements(); // else内の中身を解析
        process(); // '}'
    }
    vw.writeLabel(l2);
}

void CompilationEngine::compileDo() {
    process(); // 'do'

    // サブルーチン呼び出しの解析
	std::string name = tk->getCurrentToken();
    process(); // クラス名 または 変数名 または 関数名

    if (tk->getCurrentToken() == ".") {
        process(); // '.'
        name += ".";
		name += tk->getCurrentToken();
        process(); // 関数名
    }
    else {

    }

    process(); // '('
	int nArgs = compileExpressionList(); // 引数の解析
    process(); // ')'
    process(); // ';'

    vw.writeCall(name, nArgs);
	vw.writePop("temp", 0); // do文の戻り値は無視するのでtemp 0にpopして捨てる
}

void CompilationEngine::compileReturn() {
	process(); // 'return'

	if (tk->getCurrentToken() != ";") {
		compileExpression(); // 戻り値の式を解析
	}
	else {
		vw.writePush("constant", 0); // 戻り値がない場合は0を返す
	}
	process(); // ';'
	vw.writeReturn();
}

void CompilationEngine::compileExpression() {
    // 最初の項を解析
    compileTerm();

    // 次に演算子 (+, -, *, /, &, |, <, >, =) が続く限り、項を追加していく
    std::string t = tk->getCurrentToken();
    while (t == "+" || t == "-" || t == "*" || t == "/" || t == "&" ||
        t == "|" || t == "<" || t == ">" || t == "=") {
        std::string op = t;
        process();      // 演算子を出力
        compileTerm();  // 次の項を解析
        
		if (op == "+") vw.writeArithmetic("add");
		else if (op == "-") vw.writeArithmetic("sub");
		else if (op == "*") vw.writeCall("Math.multiply", 2);
		else if (op == "/") vw.writeCall("Math.divide", 2);
		else if (op == "&") vw.writeArithmetic("and");
		else if (op == "|") vw.writeArithmetic("or");
		else if (op == "<") vw.writeArithmetic("lt");
		else if (op == ">") vw.writeArithmetic("gt");
		else if (op == "=") vw.writeArithmetic("eq");

		t = tk->getCurrentToken(); // 次のトークンをチェック
    }
}

int CompilationEngine::compileExpressionList() {
    int nArgs = 0;
    // 次が ')' でないなら、少なくとも1つの式がある
    if (tk->getCurrentToken() != ")") {
        compileExpression();
        nArgs++;

        // 2つ目以降がある場合（カンマが続く限り）
        while (tk->getCurrentToken() == ",") {
            process(); // ','
            compileExpression();
            nArgs++;
        }
    }
    return nArgs;
}

void CompilationEngine::compileTerm() {
    TokenType type = tk->tokenType();
    std::string t = tk->getCurrentToken();

    if (type == TokenType::INT_CONST) {
        // 数値定数 push constant x
		vw.writePush("constant", tk->intVal());
		process(); // 数値はそのまま

    } else if (type == TokenType::STRING_CONST) {
        // 文字列定数
        std::string str = tk->stringVal();
        process();

		vw.writePush("constant", str.length()); // 文字列の長さをプッシュ
        vw.writeCall("string.new", 1);

        for (char c : str) {
			vw.writePush("constant", (int)c); // 文字のASCIIコードをプッシュ
            vw.writeCall("string.appendChar", 2);
        }
		
		process(); // 文字列はそのまま

    } else if (t == "true" || t == "false" || t == "null" || t == "this") {
        // キーワード定数
        if (t == "false" || t == "null") {
            vw.writePush("constant", 0);
        } else if (t == "true") {
			// true は-1なので0をpushしてからnotを呼ぶ
            vw.writePush("constant", 0);
            vw.writeArithmetic("not");
        } else if (t == "this") {
			vw.writePush("pointer", 0);
        }
		process(); // キーワード定数はそのまま
    } else if (t == "(") {
        process(); // '('
        compileExpression(); // 括弧の中はまた「式」
        process(); // ')'
    } else if (t == "-" || t == "~") {
        process(); // 単項演算子 (-x や ~true)
        compileTerm(); // その後に続く「項」を解析
        if (t == "-") vw.writeArithmetic("neg");
		if (t == "~") vw.writeArithmetic("not");

    } else if (type == TokenType::IDENTIFIER) {
        // 変数配列または関数呼び出し
        std::string name = tk->identifier();
        process();

        std::string next = tk->getCurrentToken();
        if (next == "[") {
            process(); // '['
            compileExpression();
			process(); // ']'

            // 配列のベースアドレスをプッシュ
            Kind kind = symbolTable.kindOf(name);
            int index = symbolTable.indexOf(name);
            vw.writePush(kindToSegment(kind), index);

			vw.writeArithmetic("add"); // ベースアドレス＋インデックスを計算
            vw.writePop("pointer", 1); // thatに1をセット
			vw.writePush("that", 0); // 配列の値をスタックにプッシュ

        } else if (next == "(" || next == ".") {
            // サブルーチン呼び出し
            std::string funcName = name;
			int nArgs = 0;

            if (next == ".") {
                // オブジェクト名.メソッド名あるいはクラス名.関数名
                process();
                std::string subName = tk->getCurrentToken();
                process();

                // nameがオブジェクトかどうか確認
                Kind kind = symbolTable.kindOf(name);
                if (kind != Kind::NONE) {
                    vw.writePush(kindToSegment(kind), symbolTable.indexOf(name));
                    funcName = symbolTable.typeOf(name) + "." + subName; // クラス名.メソッド名
                    nArgs = 1; // オブジェクトを引数として渡す
                }
                else {
                    funcName = name + "." + subName; // クラス名.関数名

                }
            }
            else {
				// .がない場合は同じクラス内の関数呼び出しとみなす
                vw.writePush("pointer", 0);
                funcName = className + "." + name;
				nArgs = 1;
            }

			process(); // '('
			nArgs += compileExpressionList();
			process(); // ')'
			vw.writeCall(funcName, nArgs);

        } else {
			// 変数
			Kind kind = symbolTable.kindOf(name);
			int index = symbolTable.indexOf(name);
            std::string segment = kindToSegment(kind);
            vw.writePush(segment, index);
        }
    }
}