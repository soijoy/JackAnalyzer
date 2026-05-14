#pragma once
#include <fstream>
#include <string>

class VMWriter {
public:
	// 出力先のファイルを指定して準備
	VMWriter(std::ofstream& outputFile);

	// VMコマンドを出力する関数群
	void writePush(std::string segment, int index);
	void writePop(std::string segment, int index);
	void writeArithmetic(std::string command);
	void writeLabel(std::string label);
	void writeGoto(std::string label);
	void writeIf(std::string label);
	void writeCall(std::string name, int nArgs);
	void writeFunction(std::string name, int nLocals);
	void writeReturn();

private:
	std::ofstream& out; // 出力ファイルへの参照
};