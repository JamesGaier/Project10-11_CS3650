#ifndef SYMBOLTABLE_H_
#define SYMBOLTABLE_H_
#include"typedefs.h"
#include<string>
#include<vector>
#include<map>

class symbol_table {
public:
	explicit symbol_table()=default;
	void start_subroutine();
	void define(const std::string& name, const std::string& type,
		const tds::kind knd);
	tds::u32 var_count(const tds::kind knd);
	tds::kind kind_of(const std::string& name);
	std::string type_of(const std::string& name);
	tds::u32 index_of(const std::string& name);
private:
	// name, symbol
	std::vector<std::map<std::string, tds::symbol>> table;
};


#endif
