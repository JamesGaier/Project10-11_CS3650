#include"CompilationEngine.h"
#include"typedefs.h"
#include<fstream>
#include<iostream>
#include<sstream>
#include<string>
#include <cstring>
void compilation_engine::write_op(const char op, const bool unary) {

}
tds::segment get_segment(const tds::kind knd) {
    switch (knd) {
    case tds::kind::var:
        return tds::segment::local;
    case tds::kind::arg:
        return tds::segment::arg;
    case tds::kind::field:
        return tds::segment::this_;
    case tds::kind::static_:
        return tds::segment::static_;
    }
}
compilation_engine::compilation_engine(const std::string& input_file,
                    const std::string& output_path_): output_path{output_path_} {
    output_file.open(output_path.c_str());
    std::cout << output_path.substr(0, output_path.find(".") + 1) + "vm" << std::endl;
    code_gen.init(output_path.substr(0, output_path.find(".")+1) + "vm");
    std::ifstream input{input_file};
    if(!input) {
        std::cout << "Cannot find file in directory" << std::endl;
        return;
    }
    std::stringstream ss;
    std::string line;
    while(getline(input, line)) {
        ss << line;
    }
    input_code = ss.str();

    tokens = get_tokens();
    for(const auto& el: tokens) {
        //std::cout << el.lexeme << std::endl;
    }
}
std::vector<tds::token> compilation_engine::get_tokens() {
    std::stringstream ss;
    auto done = false;
    tds::u32 idx = 0;
    tds::u32 restart = 0;
    std::vector<tds::token> tokens;
    while(input_code[restart] != '>') {
        restart++;
    }
    restart++;
    idx = restart;
    while(idx < input_code.length()) {
        idx = restart;
        while(isspace(input_code[idx])) idx++;
        restart = idx;
        // </tokens>
        if(input_code[idx] == '<' && input_code[idx+1] == '/') {
            auto temp = restart;
            temp+=2;
            while(input_code[temp] != '>') {
                ss << input_code[temp];
                temp++;
            }
            if(ss.str() == "tokens") {
                break;
            }
            ss.str("");
        }
        tds::token tok;
        // starting tag
        if(input_code[idx] == '<') {
            restart++;
            while(input_code[restart] != '>') {
                ss << input_code[restart];
                restart++;
            }
            auto type = ss.str();
            //std::cout << type << std::endl;
            if(type == "keyword") {
                tok.type = tds::token_type::keyword;
            }
            else if(type == "symbol") {
                tok.type = tds::token_type::symbol;
            }
            else if(type == "identifier") {
                tok.type = tds::token_type::identifier;
            }
            else if(type == "integerConstant") {
                tok.type = tds::token_type::integer_constant;
            }
            else if(type == "stringConstant") {
                tok.type = tds::token_type::string_constant;
            }
            restart++;
            idx = restart;
            ss.str("");
        }
        // tag contents
        if(input_code[restart] != '<') {
            while(input_code[restart] != '<') {
                ss << input_code[restart++];
            }
            tok.lexeme = ss.str();
            ss.str("");
            idx = restart;
        }
        // ending tag
        if(input_code[idx] == '<' && input_code[idx+1] == '/') {
            while(input_code[restart] != '>') restart++;
            restart++;
            idx = restart;
        }
        tokens.push_back(std::move(tok));
    }
    return tokens;
}

std::string get_tok_type(tds::token_type type) {
    switch(type) {
        case tds::token_type::keyword:
            return "keyword";
        case tds::token_type::symbol:
            return "symbol";
        case tds::token_type::identifier:
            return "identifier";
        case tds::token_type::integer_constant:
            return "integerConstant";
        case tds::token_type::string_constant:
            return "stringConstant";
        default:
            return "";
    }
}
void compilation_engine::advance() {
    if(comp_idx < tokens.size()) {
        comp_idx++;
    }
}
void compilation_engine::match(const std::string& expected) {
    if(expected == tokens[comp_idx].lexeme){
        if (tokens[comp_idx].lexeme == "field") {
            id_type = tds::identifier_type::field;
            cur_kind = tds::kind::field;
        }
        else if (tokens[comp_idx].lexeme == "static") {
            id_type = tds::identifier_type::static_;
            cur_kind = tds::kind::static_;
        }
        else if (tokens[comp_idx].lexeme == "var") {
            id_type = tds::identifier_type::var;
            cur_kind = tds::kind::var;
        }
        else if (tokens[comp_idx].lexeme == "let") {
            id_type = tds::identifier_type::let;
        }
        write_to_file();
        advance();
    }
    else {
        std::cout << "syntax error expected " << expected << " got "
            << tokens[comp_idx].lexeme << std::endl;
        syntax_error = true;
    }
}
void compilation_engine::write_to_file() {
    //std::cout << tokens[comp_idx].lexeme << std::endl;
    auto tok_type = get_tok_type(tokens[comp_idx].type);
    output_file  << '<' << tok_type << '>' 
                << tokens[comp_idx].lexeme
                << "</" << tok_type << ">\n";
}
void compilation_engine::identifier() {
    if(tokens[comp_idx].type == tds::token_type::identifier) {
        cur_name = tokens[comp_idx].lexeme;
        write_to_file();
        advance();
    }
    else {
        std::cout << "expected identifier got " << tokens[comp_idx].lexeme << std::endl;
        syntax_error = true;
    }
}
bool compilation_engine::is_type() {
    const auto path = output_path.substr(0, output_path.find_last_of("\\")+1);
    const std::ifstream f1{path+tokens[comp_idx].lexeme+".jack"};
    const std::ifstream f2{ path + tokens[comp_idx].lexeme + ".vm" };
    const auto is_primitive = primitives.count(tokens[comp_idx].lexeme) != 0;
    const auto in_file = !f1.fail() || !f2.fail();
    return in_file || is_primitive;
}
void compilation_engine::type() {
    if(is_type()) {
        cur_type = tokens[comp_idx].lexeme;
        write_to_file();
        advance();
    }
    else {
        std::cout << "type not found " << get_tok_type(tokens[comp_idx].type) << std::endl;
    }
}
void compilation_engine::compile_class() {
    output_file << "<class>\n";
    match("class");
    id_type = tds::identifier_type::class_;
    class_name = tokens[comp_idx].lexeme;
    identifier();
    match("{");
    while(tokens[comp_idx].lexeme != "}") {
        if (tokens[comp_idx].lexeme == "static"
            || tokens[comp_idx].lexeme == "field") {
            
            compile_class_var_dec();
        }
        if (tokens[comp_idx].lexeme == "constructor"
            || tokens[comp_idx].lexeme == "function"
            || tokens[comp_idx].lexeme == "method") {
            id_type = tds::identifier_type::subroutine;
            compile_subroutine();

        }
    }
    match("}");
    output_file << "</class>";
}
void compilation_engine::compile_class_var_dec() {
    output_file << "<classVarDec>\n";


    //std::cout << tokens[comp_idx].lexeme << std::endl;
    match(tokens[comp_idx].lexeme);
    compile_class_var_list(); 
    match(";");
    output_file << "</classVarDec>\n";

}
void compilation_engine::compile_subroutine() {
    output_file << "\t<subroutineDec>\n";
    table.start_subroutine();
    if (tokens[comp_idx].lexeme == "method") {
        table.define("this", class_name, tds::kind::arg);
        num_args++;
    } 
    else if (tokens[comp_idx].lexeme == "constructor") {
        is_constructor = true;
    }
    match(tokens[comp_idx].lexeme);
    if(tokens[comp_idx].lexeme == "void") {
        match("void");
    }
    else if(is_type()) {
        type();
    }
    sub_name = tokens[comp_idx].lexeme;
    identifier();
    match("(");
    output_file << "<parameterList>\n";
    if(tokens[comp_idx].lexeme != ")") {
        id_type = tds::identifier_type::arg;
        cur_kind = tds::kind::arg;
        compile_parameter_list();
    }
    table.define_signature(sub_name, num_args);
    num_args = 0;
    
    output_file << "</parameterList>\n";
    match(")");
    compile_subroutine_body();

    output_file << "\t</subroutineDec>\n";
}
void compilation_engine::compile_var_list() {
    identifier();
    if(cur_kind == tds::kind::field || cur_kind == tds::kind::static_ 
        || cur_kind == tds::kind::var)
        table.define(cur_name, cur_type, cur_kind);
    if (cur_kind == tds::kind::field) num_fields++;
    if(tokens[comp_idx].lexeme == ",") {
        match(",");
        compile_var_list();
    }
}
void compilation_engine::compile_class_var_list() {
    type();
    identifier();
    if (cur_kind == tds::kind::field) num_fields++;
    table.define(cur_name, cur_type, cur_kind);
    if (tokens[comp_idx].lexeme == ",") {
        match(",");
        compile_var_list();
    }
}
void compilation_engine::compile_var_dec() {
    output_file  << "<varDec>\n";
    match("var");
    type();
    compile_var_list();
    match(";");
    output_file  << "</varDec>\n";
    if(tokens[comp_idx].lexeme == "var") {
        compile_var_dec();
    }
}
void compilation_engine::compile_subroutine_body() {
    output_file << "<subroutineBody>\n";
    match("{");
    if (tokens[comp_idx].lexeme == "var") {
        compile_var_dec();
    }
    code_gen.write_function_header(class_name + "." + sub_name, table.var_count(tds::kind::var));
    if (is_constructor) {
        code_gen.write_push(tds::segment::const_, num_fields);
        code_gen.write_function("Memory.alloc", 1);
        code_gen.write_pop(tds::segment::pointer, 0);
        num_fields = 0;
    }
    if (table.type_of("this") == class_name) {
        code_gen.write_push(tds::segment::arg, 0);
        code_gen.write_pop(tds::segment::pointer, 0);
    }
    if (tokens[comp_idx].lexeme == "do" || tokens[comp_idx].lexeme == "let"
        || tokens[comp_idx].lexeme == "while" || tokens[comp_idx].lexeme == "return"
        || tokens[comp_idx].lexeme == "if") {
        
        compile_statements();
        
    }
    match("}");

    output_file << "</subroutineBody>\n";
}
void compilation_engine::compile_statements() {
    output_file << "<statements>\n";
    while(tokens[comp_idx].lexeme != "}") {
        if(tokens[comp_idx].lexeme == "do") {
            compile_do();
        }
        else if(tokens[comp_idx].lexeme == "let") {
            compile_let();
        }
        else if(tokens[comp_idx].lexeme == "while") {
            compile_while();
        }
        else if(tokens[comp_idx].lexeme == "return") {
            compile_return();
        }
        else if(tokens[comp_idx].lexeme == "if") {
            compile_if();
        }
    }
    output_file << "</statements>\n";
}
void compilation_engine::compile_let() {
    output_file <<  "<letStatement>\n";
    match("let");
    identifier();
    auto dest = cur_name;
    auto array_access = false;
    if(tokens[comp_idx].lexeme == "[") {
        match("[");
        compile_expression();
        code_gen.write_push(get_segment(table.kind_of(dest)),0);
        code_gen.write_arithmetic(tds::command::add);
        match("]");
        array_access = true;
    }
    match("=");
    compile_expression();
    if (!array_access) {
        code_gen.write_pop(get_segment(table.kind_of(dest)), table.index_of(dest));
    }
    else {
        code_gen.write_pop(tds::segment::temp, 0);
        code_gen.write_pop(tds::segment::pointer, 1);
        code_gen.write_push(tds::segment::temp, 0);
        code_gen.write_pop(tds::segment::that, 0);
    }
    match(";");
    output_file <<  "</letStatement>\n";
}
bool compilation_engine::is_operator() {
    return operators.count(tokens[comp_idx].lexeme) != 0;
}
void compilation_engine::consume_op() {
    auto op_found = false;
    std::string oper = "";
    for(const auto& op: operators) {
        if(op == tokens[comp_idx].lexeme) {
            op_found = true;
            oper = tokens[comp_idx].lexeme;
        }
    }
    if(op_found) {
        if (oper == "&lt;") cur_op = '<';
        else if (oper == "&gt;") cur_op = '>';
        else cur_op = tokens[comp_idx].lexeme[0];
        write_to_file();
        advance();
    }
}
void compilation_engine::compile_expression() {
    output_file  << "<expression>\n";
    auto unary = false;
    if (tokens[comp_idx].lexeme[0] == '-' || tokens[comp_idx].lexeme[0] == '~') {
        unary = true;
    }
    compile_term();
    while(is_operator()) {
        consume_op();
        compile_term();
        switch (cur_op) {
            case '+':
                code_gen.write_arithmetic(tds::command::add);
                break;
            case '-':
                code_gen.write_arithmetic((unary) ? tds::command::neg : tds::command::sub);
                break;
            case '=':
                code_gen.write_arithmetic(tds::command::eq);
                break;
            case '<':
                code_gen.write_arithmetic(tds::command::lt);
                code_gen.write_arithmetic(tds::command::not_);
                break;
            case '>':
                code_gen.write_arithmetic(tds::command::lt);
                break;
            case '*':
                code_gen.write_function("Math.multiply", 2);
                break;
            case '/':
                code_gen.write_function("Math.divide", 2);
                break;
            default:
                std::cout << "Unimplemented operator: " << cur_op << std::endl;
                break;
        }
    }


    output_file << "</expression>\n";
}

void compilation_engine::compile_term() {
    output_file  << "<term>\n";
    auto kind = table.kind_of(tokens[comp_idx].lexeme);
    if(tokens[comp_idx].type == tds::token_type::integer_constant) {
        code_gen.write_push(tds::segment::const_, static_cast<tds::u32>(stoi(tokens[comp_idx].lexeme)));
        match(tokens[comp_idx].lexeme);
    }
    else if(tokens[comp_idx].type == tds::token_type::string_constant) {
        code_gen.write_push(tds::segment::const_, tokens[comp_idx].lexeme.size());
        code_gen.write_function("String.new", 1);
        auto cur_str = tokens[comp_idx].lexeme;
        for (const auto& ch : cur_str) {
            tds::u32 num_ch = static_cast<tds::u32>(ch);
            code_gen.write_push(tds::segment::const_, num_ch);
            code_gen.write_function("String.appendChar", 2);
        }
        match(tokens[comp_idx].lexeme);
    }
    else if(tokens[comp_idx].type == tds::token_type::keyword) {
        if (tokens[comp_idx].lexeme == "false") {
            code_gen.write_push(tds::segment::const_, 0);
        }
        else if (tokens[comp_idx].lexeme == "true") {
            code_gen.write_push(tds::segment::const_, 0);
            code_gen.write_arithmetic(tds::command::not_);
        }
        match(tokens[comp_idx].lexeme);
    }
    else if(tokens[comp_idx].lexeme == "(") {
        match("(");
        compile_expression();
        match(")");
    }
    else if(tokens[comp_idx].lexeme == "-" || tokens[comp_idx].lexeme == "~") {
        consume_op();
        
        compile_term();
    }
    else if (tokens[comp_idx].type == tds::token_type::identifier
            && tokens[comp_idx+1].lexeme != "(" && tokens[comp_idx+1].lexeme != "."){
        code_gen.write_push(get_segment(table.kind_of(tokens[comp_idx].lexeme)), table.index_of(tokens[comp_idx].lexeme));
        identifier();
        if (tokens[comp_idx].lexeme == "[") {
            match("[");
            compile_expression();
            code_gen.write_arithmetic(tds::command::add);
            code_gen.write_pop(tds::segment::pointer, 1);
            code_gen.write_push(tds::segment::that, 0);
            match("]");
        }
    }
    compile_subroutine_call();
    output_file << "</term>\n";
}
void compilation_engine::compile_expression_list() {
    output_file <<  "<expressionList>\n";
    if (tokens[comp_idx].lexeme != ")") {
        compile_expression();
        num_args++;
        while (tokens[comp_idx].lexeme == ",") {
            match(",");
            compile_expression();
            num_args++;
        }
    }
    output_file <<  "</expressionList>\n";
}
void compilation_engine::compile_while() {
    output_file << "<whileStatement>\n";
    match("while");
    match("(");
    code_gen.write_label("WHILE_EXP" + std::to_string(while_counter));
    compile_expression();
    match(")");
    match("{");
    code_gen.write_if("WHILE_END" + std::to_string(while_end_counter));
    compile_statements();
    code_gen.write_goto("WHILE_EXP" + std::to_string(while_counter));
    code_gen.write_label("WHILE_END" + std::to_string(while_end_counter));
    match("}");
    output_file << "</whileStatement>\n";
}
void compilation_engine::compile_subroutine_call() {
    if (is_type()) {
        auto type_name = tokens[comp_idx].lexeme;
        type();
        match(".");
        auto func_name = tokens[comp_idx].lexeme;
        identifier();
        match("(");
        compile_expression_list();
        match(")");
        if(table.contains_signature(func_name))
            code_gen.write_function(class_name + "." + func_name, table.num_args(func_name)); 
        else
            code_gen.write_function(type_name + "." + func_name, num_args);
        num_args = 0;
    }
    else if (tokens[comp_idx].type == tds::token_type::identifier) {
        code_gen.write_push(get_segment(table.kind_of(tokens[comp_idx].lexeme)), table.index_of(tokens[comp_idx].lexeme));
        identifier();
        if (tokens[comp_idx].lexeme == "(") {
            match("(");
            compile_expression_list();
            match(")");

        }
        else if (tokens[comp_idx].lexeme == ".") {
            match(".");
            std::string func_name = tokens[comp_idx].lexeme;
            identifier();
            match("(");
            compile_expression_list();
            match(")");
            code_gen.write_function(class_name +"."+ func_name, table.num_args(func_name));

            
        }

    }
}
void compilation_engine::compile_do() {
    output_file << "<doStatement>\n";
    match("do");
    compile_subroutine_call();
    code_gen.write_pop(tds::segment::temp, 0);
    match(";");
    output_file << "</doStatement>\n";
}
void compilation_engine::compile_return() {
    output_file << "<returnStatement>\n";
    match("return");
    if(tokens[comp_idx].lexeme != ";") {
        compile_expression();
        if (is_constructor) {
            code_gen.write_push(tds::segment::pointer, 0);
            is_constructor = false;
        }
        code_gen.write_return();
    }
    else {
        code_gen.write_push(tds::segment::const_, 0);
        code_gen.write_return();
    }
    match(";");
    output_file << "</returnStatement>\n";
}
void compilation_engine::compile_if() {
    output_file << "<ifStatement>\n";
    match("if");
    match("(");
    compile_expression();
    code_gen.write_if("IF_TRUE" + std::to_string(if_true_counter));
    code_gen.write_goto("IF_FALSE" + std::to_string(if_false_counter));
    match(")");
    match("{");
    code_gen.write_label("IF_TRUE" + std::to_string(if_true_counter));
    compile_statements();
    
    match("}");
    
    if (tokens[comp_idx].lexeme == "else") {
        code_gen.write_goto("IF_END" + std::to_string(if_end_counter));
        code_gen.write_label("IF_FALSE" + std::to_string(if_false_counter));
        match("else");
        match("{");
        
        compile_statements();
        
        match("}");
        code_gen.write_label("IF_END" + std::to_string(if_end_counter));
    }
    else {
        code_gen.write_label("IF_FALSE" + std::to_string(if_false_counter));
    }
    if_true_counter++;
    if_false_counter++;
    if_end_counter++;
    output_file << "</ifStatement>\n";
}
void compilation_engine::compile_parameter_list() {
    type();
    identifier();
    if (cur_kind == tds::kind::arg) {
        table.define(cur_name, cur_type, cur_kind);
        num_args++;
    }
    if(tokens[comp_idx].lexeme == ",") {
        match(",");
        compile_parameter_list();
    }
}
void compilation_engine::debug_print() {
    table.debug_print();
}
