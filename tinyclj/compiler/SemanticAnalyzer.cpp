#include <unordered_map>
#include <iostream>

#include "compiler/ast/BodyExpr.h"
#include "compiler/ast/BooleanExpr.h"
#include "compiler/ast/CapturedBindingExpr.h"
#include "compiler/ast/CharExpr.h"
#include "compiler/ast/DefExpr.h"
#include "compiler/ast/DoubleExpr.h"
#include "compiler/ast/FunctionExpr.h"
#include "compiler/ast/IfExpr.h"
#include "compiler/ast/IntegerExpr.h"
#include "compiler/ast/InvokeExpr.h"
#include "compiler/ast/LetExpr.h"
#include "compiler/ast/NilExpr.h"
#include "compiler/ast/QuotedExpr.h"
#include "compiler/ast/ScopedLocalBindingExpr.h"
#include "compiler/ast/StringExpr.h"
#include "compiler/ast/VarExpr.h"
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

void captureLocalBinding(CompilerContext &ctx, const std::string &name) {
    for (FunctionFrame *currentFrame = ctx.m_CurrentFunctionFrame;
         currentFrame;
         currentFrame = currentFrame->m_ParentFrame) {
        // resolve local var
        if (currentFrame->m_Locals.contains(name)) {
            return;
        }
        // already captured var
        if (auto capture = currentFrame->m_Captures.find(name); capture != currentFrame->m_Captures.end()) {
            return;
        }
        // not captured -> capture recursively
        currentFrame->m_Captures.emplace(name, currentFrame->m_Captures.size());
    }
}

AExpr resolveSymbol(CompilerContext &ctx, const Object *form) {
    std::string name = tc_symbol_valueX(form);
    // Step 1: Try local bindings (locals can override globals)
    if (ctx.m_LocalBindings.contains(name)) {
        captureLocalBinding(ctx, name);
        FunctionFrame *currentFrame = ctx.m_CurrentFunctionFrame;
        if (currentFrame->m_Locals.contains(name)) {
            return std::make_unique<ScopedLocalBindingExpr>(name);
        } else {
            return std::make_unique<CapturedBindingExpr>(name, currentFrame->m_Captures.at(name));
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
