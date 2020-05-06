#ifndef TYPEDEFS_H_
#define TYPEDEFS_H_
#include<cstdint>
#include<string>
#include<map>
namespace tds {
    using u64 = uint64_t;
    using i64 = int64_t;
    using u32 = uint32_t;
    using i32 = int32_t;
    using u16 = uint16_t;
    using i16 = int16_t;
    enum struct keywords {
        class_, method, function,
        constructor, int_, boolean,
        char_, void_, var, static_, field, let,
        do_, if_, else_, while_, return_, true_, false_,
        null, this_
    };
    enum struct token_type {
        keyword, symbol, identifier, integer_constant,
        string_constant
    };
    enum struct kind {
        static_, field, arg, var, none
    };
    enum struct segment {
        const_, arg, local, static_, this_,
        that, pointer, temp, none
    };
    enum struct command {
        add, sub, neg, eq, gt, lt,
        and_, or_, not_, mul, div
    };
    enum struct identifier_type {
        static_, field, arg, var, class_, subroutine, let, none
    };
    struct token {
        token_type type;
        std::string lexeme;
    };
    struct symbol {
        std::string type;
        kind kind;
        u32 index;
    };

}
#endif