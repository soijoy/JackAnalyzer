#pragma once
#include <fstream>
#include <string>

class VMWriter {
public:
	// Construct with output file reference
	explicit VMWriter(std::ofstream& outputFile);

	// VM command output functions
	void writePush(const std::string& segment, int index);
	void writePop(const std::string& segment, int index);
	void writeArithmetic(const std::string& command);
	void writeLabel(const std::string& label);
	void writeGoto(const std::string& label);
	void writeIf(const std::string& label);
	void writeCall(const std::string& name, int nArgs);
	void writeFunction(const std::string& name, int nLocals);
	void writeReturn();

private:
	std::ofstream& out;
};
