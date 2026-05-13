#include "VMWriter.h"

void VMWriter::writePush(std::string segment, int index) {
	out << "push" << segment << " " << index << std::endl;
}