#ifndef SYMBOLTABLE_H_
#define SYMBOLTABLE_H_
#include"typedefs.h"
#include<string>
#include<vector>
#include<map>
#include<iostream>
class symbol_table {
public:
	explicit symbol_table();
	void start_subroutine();
	void define(const std::string& name, const std::string& type,
		const tds::kind knd);
	tds::u32 var_count(const tds::kind knd);
	tds::kind kind_of(const std::string& name);
	std::string type_of(const std::string& name);
	tds::u32 index_of(const std::string& name);
	void debug_print();
	tds::u32 num_args(const std::string& name);
	void define_signature(const std::string& name, const tds::u32 nargs);
	bool contains_signature(const std::string& name);
private:
	// name, symbol
	std::vector<std::map<std::string, tds::symbol>> table;
	std::map<std::string, tds::u32> signatures;
};


#endif
