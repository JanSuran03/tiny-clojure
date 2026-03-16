#include <unordered_map>
#include <iostream>

#include "BodyExpr.h"
#include "BooleanExpr.h"
#include "CharExpr.h"
#include "DefExpr.h"
#include "DoubleExpr.h"
#include "FunctionExpr.h"
#include "IfExpr.h"
#include "IntegerExpr.h"
#include "InvokeExpr.h"
#include "LetExpr.h"
#include "LocalBindingExpr.h"
#include "NilExpr.h"
#include "QuotedExpr.h"
#include "StringExpr.h"
#include "VarExpr.h"
#include "SemanticAnalyzer.h"
#include "Runtime.h"
#include "types/TCBoolean.h"
#include "types/TCChar.h"
#include "types/TCDouble.h"
#include "types/TCInteger.h"
#include "types/TCList.h"
#include "types/TCString.h"
#include "types/TCSymbol.h"
#include "types/TCVar.h"

using AnalyzerFn = AExpr (*)(ExpressionMode mode, CompilerContext &ctx, const Object *form);

std::unordered_map<std::string, AnalyzerFn> m_SpecialAnalyzers = {
        {"def",   DefExpr::parse},
        {"if",    IfExpr::parse},
        {"do",    BodyExpr::parse},
        {"quote", QuotedExpr::parse},
        {"let*",  LetExpr::parse},
        {"fn*",   FunctionExpr::parse},
};

AExpr resolveSymbol(CompilerContext &ctx, const Object *form) {
    std::string name = tc_symbol_valueX(form);
    // Step 1: Try local bindings (locals can override globals)
    if (ctx.m_LocalBindings.contains(name)) {
        for (FunctionFrame *currentFrame = ctx.m_CurrentFunctionFrame;
             currentFrame;
             currentFrame = currentFrame->m_ParentFrame) {
            // local var or already captured
            if (currentFrame->m_Locals.contains(name) ||
                currentFrame->m_Captures.contains(name)) {
                return std::make_unique<LocalBindingExpr>(name);
            }
            // not captured -> capture recursively
            currentFrame->m_Captures.emplace(name, currentFrame->m_Captures.size());
        }
    }
    // Step 2: Try global vars
    if (TCVar *var = ctx.m_RuntimeRef.getVar(name)) {
        return std::make_unique<VarExpr>(var);
    }
    throw std::runtime_error(std::string("Cannot resolve symbol: ").append(name).append(" in the context"));
}

AExpr SemanticAnalyzer::analyze(CompilerContext &ctx, const Object *form) {
    return analyze(ExpressionMode::EXPRESSION, ctx, form);
}

AExpr SemanticAnalyzer::analyze(ExpressionMode mode, CompilerContext &ctx, const Object *form) {
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

            if (head == nullptr) {
                throw std::runtime_error("Cannot call nil.");
            }

            if (auto ana_it = m_SpecialAnalyzers.find(tc_symbol_valueX(head)); ana_it != m_SpecialAnalyzers.end()) {
                return ana_it->second(mode, ctx, form);
            }

            // todo: macroexpansion
            return InvokeExpr::parse(mode, ctx, form);
        }
        case ObjectType::CHARACTER:
            return std::make_unique<CharExpr>(tc_char_valueX(form));
        case ObjectType::STRING:
            return std::make_unique<StringExpr>(tc_string_valueX(form));
        case ObjectType::SYMBOL:
            return resolveSymbol(ctx, form);
        case ObjectType::FUNCTION:
            throw std::runtime_error("Functions are not supported yet");
        case ObjectType::CLOSURE:
            throw std::runtime_error("Closures are not supported yet");
    }
    std::unreachable();
}
