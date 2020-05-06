#include"VMWriter.h"
#include<string>
#include<iostream>
void vm_writer::init(const std::string& output_path) {
	output_file.open(output_path);
}
std::string segtostr(const tds::segment sgmt) {
	switch (sgmt) {
		case tds::segment::arg:
			return "argument";
		case tds::segment::const_:
			return "constant";
		case tds::segment::local:
			return "local";
		case tds::segment::pointer:
			return "pointer";
		case tds::segment::static_:
			return "static";
		case tds::segment::temp:
			return "temp";
		case tds::segment::that:
			return "that";
		case tds::segment::this_:
			return "this";
		default:
			std::cout << "Could not find segment number: " << static_cast<tds::u32>(sgmt) << std::endl;
			return "";
	}
}
std::string cmdtostr(const tds::command cmd) {
	switch (cmd) {
		case tds::command::add:
			return "add";
		case tds::command::and_:
			return "and";
		case tds::command::eq:
			return "eq";
		case tds::command::gt:
			return "gt";
		case tds::command::lt:
			return "lt";
		case tds::command::neg:
			return "neg";
		case tds::command::not_:
			return "not";
		case tds::command::or_:
			return "or";
		case tds::command::sub:
			return "sub";
		default:
			return "";
	}
}
void vm_writer::write_push(const tds::segment sgmt, const  tds::u32 idx) {
	
	output_file << "push " <<  segtostr(sgmt) << " " << idx << "\n";
}
void vm_writer::write_pop(const tds::segment sgmt, const  tds::u32 idx) {
	output_file << "pop " << segtostr(sgmt) << " " << idx << "\n";
}
void vm_writer::write_arithmetic(const tds::command cmd) {
	output_file << cmdtostr(cmd) << "\n";
}
void vm_writer::write_label(const std::string& label) {
	output_file << "label " << label << "\n";
}
void vm_writer::write_goto(const std::string& label) {
	output_file << "goto " << label << "\n";
}
void vm_writer::write_if(const std::string& label) {
	output_file << "if-goto " << label << "\n";
}
void vm_writer::write_function(const std::string& label, const tds::u32 nargs) {
	output_file << "call " <<  label << " " << nargs << "\n";
}
void vm_writer::write_function_header(const std::string& label, const tds::u32 size) {
	output_file << "function " << label << " " << size << "\n";
}
void vm_writer::write_return() {
	output_file << "return" << "\n";
}
void vm_writer::close() {
	output_file.close();
}