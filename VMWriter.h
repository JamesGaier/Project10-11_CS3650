#ifndef VMWRITER_H_
#define VMWRITER_H_
#include"typedefs.h"
#include<string>
#include<fstream>

class vm_writer {
public:
	void init(const std::string& output_file);
	void write_push(tds::segment sgmt, const tds::u32 index);
	void write_pop(tds::segment sgmt, const  tds::u32 index);
	void write_arithmetic(tds::command cmd);
	void write_label(const std::string& label);
	void write_goto(const std::string& label);
	void write_if(const std::string& label);
	void write_function(const std::string& name, const  tds::u32 nargs);
	void write_function_header(const std::string& name, const  tds::u32 size);
	void write_return();
	void close();
private:
	std::ofstream output_file;
	std::string class_name = "";
};

#endif
