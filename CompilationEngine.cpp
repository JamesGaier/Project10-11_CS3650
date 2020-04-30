#include"CompilationEngine.h"
#include"typedefs.h"
#include<fstream>
#include<iostream>
#include<sstream>
#include<string>
#include <cstring>
compilation_engine::compilation_engine(const std::string& input_file,
                    const std::string& output_path_): output_path{output_path_} {

    output_file.open(output_path.c_str());
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
        std::cout << el.lexeme << std::endl;
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
    const std::ifstream file{path+tokens[comp_idx].lexeme+".jack"};
    const auto is_primitive = primitives.count(tokens[comp_idx].lexeme) != 0;
    const auto in_file = !file.fail();
    return in_file || is_primitive;
}
void compilation_engine::type() {
    if(is_type()) {
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
            compile_subroutine();
        }
    }
    match("}");
    output_file << "</class>";
}
void compilation_engine::compile_class_var_dec() {
    output_file << "<classVarDec>\n";
    match(tokens[comp_idx].lexeme);
    compile_class_var_list(); 
    match(";");
    output_file << "</classVarDec>\n";

}
void compilation_engine::compile_subroutine() {
    output_file << "\t<subroutineDec>\n";

    match(tokens[comp_idx].lexeme);
    if(tokens[comp_idx].lexeme == "void") {
        match("void");
    }
    else if(is_type()) {
        type();
    }
    identifier();
    match("(");
    output_file << "<parameterList>\n";
    if(tokens[comp_idx].lexeme != ")") {
        compile_parameter_list();
    }
    output_file << "</parameterList>\n";
    match(")");
    compile_subroutine_body();
    output_file << "\t</subroutineDec>\n";
}
void compilation_engine::compile_var_list() {
    identifier();
    if(tokens[comp_idx].lexeme == ",") {
        match(",");
        compile_var_list();
    }
}
void compilation_engine::compile_class_var_list() {
    type();
    identifier();
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
    if(tokens[comp_idx].lexeme == "[") {
        match("[");
        compile_expression();
        match("]");
    }
    match("=");
    compile_expression();
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
        write_to_file();
        advance();
    }
}
void compilation_engine::compile_expression() {
    output_file  << "<expression>\n";
    compile_term();
    while(is_operator()) {
        consume_op();
        compile_term();
    }
    output_file << "</expression>\n";
}
void compilation_engine::compile_term() {
    output_file  << "<term>\n";
    if(tokens[comp_idx].type == tds::token_type::integer_constant) {
        match(tokens[comp_idx].lexeme);
    }
    else if(tokens[comp_idx].type == tds::token_type::string_constant) {
        match(tokens[comp_idx].lexeme);

    }
    else if(tokens[comp_idx].type == tds::token_type::keyword) {
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
    compile_subroutine_call();
    output_file << "</term>\n";
}
void compilation_engine::compile_expression_list() {
    output_file <<  "<expressionList>\n";
    if (tokens[comp_idx].lexeme != ")") {
        compile_expression();
        while (tokens[comp_idx].lexeme == ",") {
            match(",");
            compile_expression();
        }
    }
    output_file <<  "</expressionList>\n";
}
void compilation_engine::compile_while() {
    output_file << "<whileStatement>\n";
    match("while");
    match("(");
    compile_expression();
    match(")");
    match("{");

    compile_statements();
    
    match("}");
    output_file << "</whileStatement>\n";
}
void compilation_engine::compile_subroutine_call() {
    if (tokens[comp_idx].type == tds::token_type::identifier) {
        identifier();
        if (tokens[comp_idx].lexeme == "(") {
            match("(");
            compile_expression_list();
            match(")");
        }
        else if (tokens[comp_idx].lexeme == ".") {
            match(".");
            identifier();
            match("(");
            compile_expression_list();
            match(")");
        }
        else if (tokens[comp_idx].lexeme == "[") {
            match("[");
            compile_expression();
            match("]");
        }
    }
    else if (is_type()) {
        match(".");
        identifier();
        match("(");
        compile_expression_list();
        match(")");
    }
}
void compilation_engine::compile_do() {
    output_file << "<doStatement>\n";
    match("do");
    compile_subroutine_call();
    match(";");
    output_file << "</doStatement>\n";
}
void compilation_engine::compile_return() {
    output_file << "<returnStatement>\n";
    match("return");
    if(tokens[comp_idx].lexeme != ";") {
        compile_expression();
    }
    match(";");
    output_file << "</returnStatement>\n";
}
void compilation_engine::compile_if() {
    output_file << "<ifStatement>\n";
    match("if");
    match("(");
    compile_expression();
    match(")");
    match("{");
    
    compile_statements();
    
    match("}");
    if (tokens[comp_idx].lexeme == "else") {
        match("else");
        match("{");
        
        compile_statements();
        
        match("}");
    }
    output_file << "</ifStatement>\n";
}
void compilation_engine::compile_parameter_list() {
    type();
    identifier();
    if(tokens[comp_idx].lexeme == ",") {
        match(",");
        compile_parameter_list();
    }
}