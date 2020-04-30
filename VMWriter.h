#ifndef VMWRITER_H_
#define VMWRITER_H_
#include<string>
#include"typedefs.h"
class vm_writer {
public:
	explicit vm_writer(const std::string& output_path);
	void write_push(tds::segment sgmt, const int index);
	void write_pop(tds::segment sgmt, const int index);
	void write_arithmetic(tds::command cmd);
	void write_label(const std::string& label);
	void write_goto(const std::string& label);
	void write_if(const std::string& label);
	void write_function(const std::string& name, const int nargs);
	void write_return();
	void close();

};

#endif
