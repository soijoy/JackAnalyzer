#include "VMWriter.h"

VMWriter::VMWriter(std::ofstream& outputFile) : out(outputFile) {
}

void VMWriter::writePush(std::string segment, int index) {
	out << "push " << segment << " " << index << std::endl;
}

void VMWriter::writePop(std::string segment, int index) {
    out << "pop " << segment << " " << index << std::endl;
}

void VMWriter::writeArithmetic(std::string command) {
    // add, sub, neg, eq, gt, lt, and, or, not
    out << command << std::endl;
}

void VMWriter::writeLabel(std::string label) {
    out << "label " << label << std::endl;
}

void VMWriter::writeGoto(std::string label) {
    out << "goto " << label << std::endl;
}

void VMWriter::writeIf(std::string label) {
    out << "if-goto " << label << std::endl;
}

void VMWriter::writeCall(std::string name, int nArgs) {
    out << "call " << name << " " << nArgs << std::endl;
}

void VMWriter::writeFunction(std::string name, int nLocals) {
    out << "function " << name << " " << nLocals << std::endl;
}

void VMWriter::writeReturn() {
    out << "return" << std::endl;
}