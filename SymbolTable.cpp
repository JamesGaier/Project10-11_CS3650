#include"SymbolTable.h"

// pushes back one for the class
symbol_table::symbol_table() {
	table.push_back(std::map<std::string, tds::symbol>{});
}
// for functions/methods
void symbol_table::start_subroutine() {
	table.push_back(std::map<std::string, tds::symbol>{});
}
void symbol_table::define(const std::string& name, const std::string& type,
							const tds::kind knd) {
	auto & cur_table = table.back();
	cur_table.insert({ name, { type, knd, var_count(knd) } });
}
tds::u32 symbol_table::var_count(const tds::kind knd) {
	auto count = 0;
	auto cur_table = table.back();
	for (const auto& symbol : cur_table) {
		if (knd == symbol.second.kind) {
			count++;
		}
	}
	return count;
}
// ask about copy
tds::kind symbol_table::kind_of(const std::string& name) {
	auto temp = table;
	auto result = tds::kind::none;
	auto find_scope = [&]() {
		for (const auto& symbol : temp.back()) {
			if (symbol.first == name) {
				return symbol.second.kind;
			}
		}
		return tds::kind::none;
	};
	result = find_scope();
	while (temp.size() != 1) temp.pop_back();
	return (result == tds::kind::none) ? find_scope():result;
}
std::string symbol_table::type_of(const std::string& name) {
	auto temp = table;
	std::string type = "";
	auto found = false;
	auto find_scope = [&]() {
		for (const auto& scope : temp.back()) {
			if (scope.first == name) {
				found = true;
				return scope.second.type;
			}
		}
		return std::string{};
	};
	type = find_scope();
	while (temp.size() != 1) temp.pop_back();
	return (!found) ? find_scope() : type;
}
tds::u32 symbol_table::index_of(const std::string& name) {
	auto temp = table;
	tds::u32 index = 0;
	auto found = false;
	auto find_scope = [&](){
		for (const auto& scope : temp.back()) {
			if (scope.first == name) {
				found = true;
				return scope.second.index;
			}
		}
		return tds::u32(0);
	};
	index = find_scope();
	while (temp.size() != 1) temp.pop_back();
	return (!found) ? find_scope() : index;
}
std::string kndtostr(const tds::kind knd) {
	switch (knd) {
		case tds::kind::var:
			return "var";
		case tds::kind::field:
			return "field";
		case tds::kind::arg:
			return "argument";
		case tds::kind::static_:
			return "static";
		case tds::kind::none:
			return "none";
		default:
			return "none";
	}
	return "";
}
void symbol_table::debug_print() {
	for (const auto& scope : table) {
		std::cout << std::endl;
		for (const auto& symbol : scope) {
			std::cout << symbol.first << " "
				<< symbol.second.type << " "
				<< kndtostr(symbol.second.kind) << " "
				<< symbol.second.index << std::endl;
		}
	}
}
tds::u32 symbol_table::num_args(const std::string& name) {
	if (!contains_signature(name)) {
		return 0;
	}
	return signatures.at(name);
}
void symbol_table::define_signature(const std::string& name, const tds::u32 nargs) {
	signatures.emplace(name, nargs);
}
bool symbol_table::contains_signature(const std::string& name) {
	return signatures.count(name) != 0;
}