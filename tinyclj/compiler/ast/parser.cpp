#include <unordered_map>

#include "BodyExpr.h"
#include "BooleanExpr.h"
#include "CharExpr.h"
#include "DoubleExpr.h"
#include "IfExpr.h"
#include "IntegerExpr.h"
#include "LetExpr.h"
#include "NilExpr.h"
#include "QuotedExpr.h"
#include "StringExpr.h"
#include "VarExpr.h"
#include "parser.h"
#include "types/TCBoolean.h"
#include "types/TCChar.h"
#include "types/TCDouble.h"
#include "types/TCInteger.h"
#include "types/TCList.h"
#include "types/TCString.h"
#include "types/TCSymbol.h"

using AnalyzerFn = AExpr (*)(CompilerContext &ctx, const Object *form);

std::unordered_map<std::string, AnalyzerFn> m_SpecialAnalyzers = {
        {"if",    IfExpr::parse},
        {"do",    IfExpr::parse},
        {"quote", QuotedExpr::parse},
        {"let*",  LetExpr::parse},
};

AExpr Parser::analyze(CompilerContext &ctx, const Object *form) {
    if (form == nullptr) {
        return std::make_unique<NilExpr>();
    }
    switch (form->m_Type) {
        case ObjectType::BOOLEAN:
            return std::make_unique<BooleanExpr>(tc_boolean_valueX(form));
        case ObjectType::INTEGER:
            return std::make_unique<IntegerExpr>(tc_integer_valueX(form));
        case ObjectType::DOUBLE:
            return std::make_unique<DoubleExpr>(tc_double_valueX(form));
        case ObjectType::LIST: {
            if (tc_list_seq(form) == nullptr) {
                throw std::runtime_error("Empty lists are not supported yet");
            }
            const Object *head = tc_list_first(form);
            if (auto ana_it = m_SpecialAnalyzers.find(tc_symbol_valueX(head)); ana_it != m_SpecialAnalyzers.end()) {
                return ana_it->second(ctx, form);
            }


            // todo: macroexpansion, special forms, invokes
            throw std::runtime_error("Lists are not fully supported yet");
        }
        case ObjectType::CHARACTER:
            return std::make_unique<CharExpr>(tc_char_valueX(form));
        case ObjectType::STRING:
            return std::make_unique<StringExpr>(tc_string_valueX(form));
        case ObjectType::SYMBOL:
            return VarExpr::resolveVar(ctx, form);
        case ObjectType::FUNCTION:
            throw std::runtime_error("Functions are not supported yet");
    }
    std::unreachable();
}

Object *Parser::eval(CompilerContext &ctx, const Object *form) {
    // wrap the code in an anonymous function call (for now, for all forms), then evaluate that function
    const Object *new_form = tc_list_cons(tc_symbol_new("fn*"),
                                          tc_list_cons(empty_list(), form));
    AExpr expr = analyze(ctx, new_form);
    // print the llvm module
    ctx.m_Module.dump();
    return expr->eval();
}
