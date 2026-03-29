#include <unordered_map>

#include "compiler/ast/BodyExpr.h"
#include "compiler/ast/BooleanExpr.h"
#include "compiler/ast/local-binding/CapturedLocalExpr.h"
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
#include "compiler/ast/RecurExpr.h"
#include "compiler/ast/StringExpr.h"
#include "compiler/ast/VarDerefExpr.h"
#include "compiler/ast/VarLiteralExpr.h"
#include "SemanticAnalyzer.h"
#include "runtime/Runtime.h"
#include "types/TCBoolean.h"
#include "types/TCChar.h"
#include "types/TCDouble.h"
#include "types/TCInteger.h"
#include "types/TCList.h"
#include "types/TCString.h"
#include "types/TCSymbol.h"

using AnalyzerFn = AExpr (*)(ExpressionMode mode, AnalyzerContext &ctx, const Object *form);

std::unordered_map<std::string, AnalyzerFn> m_SpecialAnalyzers = {
        {"def",               DefExpr::parse},
        {"if",                IfExpr::parse},
        {"do",                BodyExpr::parse},
        {"quote",             QuotedExpr::parse},
        {"let*",              LetExpr::parse},
        {"loop*",             LetExpr::parse},
        {"fn*",               FunctionExpr::parse},
        {"__eval_fn_wrapper", FunctionExpr::parse},
        {"recur",             RecurExpr::parse},
        {"var",               VarLiteralExpr::parse}
};

bool SemanticAnalyzer::isSpecial(const Object *obj) {
    return obj
           && obj->m_Type == ObjectType::SYMBOL
           && m_SpecialAnalyzers.contains(tc_symbol_valueX(obj));
}

// Returns the local binding for the captured variable, which can be used to resolve the variable
// reference in the current frame, which is needed to load the variable value at runtime and copy
// it into the closure environment when the variable is captured.
std::shared_ptr<BindingExpr> captureLocalBindingRec(AnalyzerContext &ctx, const std::string &name, ssize_t frame_index) {
    auto &currentFrameBindings = ctx.m_StackFrameBindings[frame_index];
    auto &currentFrameCapturesMapping = ctx.m_CapturesMappingStack[frame_index];
    if (frame_index == 0) {
        return currentFrameBindings.at(name);
    }
    // already a local var -> nothing to do
    if (auto binding = currentFrameBindings.find(name); binding != currentFrameBindings.end()) {
        return binding->second;
    }

    // Mark the current frame (function overload) as using the capture environment, which means that the parent
    // function stub needs to pass the environment struct to the function overload at runtime
    if (!ctx.m_CaptureUsedStack[frame_index]) {
        ctx.m_CaptureUsedStack[frame_index] = true;
    }

    // capture if not already captured
    if (!currentFrameCapturesMapping.contains(name)) {
        auto parent_binding = captureLocalBindingRec(ctx, name, frame_index - 1);
        // Assign a slot for the captured variable in the current frame's capture mapping
        unsigned closureEnvIndex = static_cast<unsigned>(currentFrameCapturesMapping.size());
        currentFrameCapturesMapping.emplace(name, CapturedLocalExpr(name,
                                                                    closureEnvIndex,
                                                                    parent_binding));
        return std::make_shared<CapturedLocalExpr>(
                currentFrameCapturesMapping.at(name)
        );
    } else {
        // already captured -> do nothing
        return std::make_shared<CapturedLocalExpr>(
                currentFrameCapturesMapping.at(name)
        );
    }
}

void captureLocalBinding(AnalyzerContext &ctx, const std::string &name) {
    captureLocalBindingRec(ctx, name, static_cast<ssize_t>(ctx.m_StackFrameBindings.size()) - 1);
    /*for (ssize_t i = static_cast<ssize_t>(ctx.m_StackFrameBindings.size()) - 1; i >= 0; i--) {
        auto &currentFrameBindings = ctx.m_StackFrameBindings[i];
        auto &currentFrameCapturesMapping = ctx.m_CapturesMappingStack[i];
        // already a local var -> nothing to do
        if (auto binding = currentFrameBindings.find(name); binding != currentFrameBindings.end()) {
            return binding->second;
        }

        // Mark the current frame (function overload) as using the capture environment, which means that the parent
        // function stub needs to pass the environment struct to the function overload at runtime
        if (!ctx.m_CaptureUsedStack[i]) {
            ctx.m_CaptureUsedStack[i] = true;
        }

        // capture if not already captured
        if (!currentFrameCapturesMapping.contains(name)) {
            auto parent_binding = captureLocalBinding(ctx, name);
            // Assign a slot for the captured variable in the current frame's capture mapping
            unsigned closureEnvIndex = static_cast<unsigned>(currentFrameCapturesMapping.size());
            currentFrameCapturesMapping.emplace(name, CapturedLocalExpr(name,
                                                                        closureEnvIndex,
                                                                        parent_binding));
        } else {
            // already captured -> do nothing
            return std::make_shared<CapturedLocalExpr>(
                    currentFrameCapturesMapping.at(name)
            );
        }
    }*/
}

AExpr resolveSymbol(AnalyzerContext &ctx, const Object *form) {
    std::string name = tc_symbol_valueX(form);
    // Step 1: Check, whether the symbol is locally bound in the scope
    if (ctx.m_ScopeBindings.contains(name)) {
        // Recursively capture the local binding in all frames between the current frame
        // and the frame where the local binding is defined
        captureLocalBinding(ctx, name);
        if (auto local = ctx.currentStackFrameBindings().find(name); local != ctx.currentStackFrameBindings().end()) {
            // Available as a local variable in the current frame -> resolve as a local variable (or a fn arg)
            return local->second->clone();
        } else {
            // Available as a captured variable in the current frame -> resolve as a captured variable
            return ctx.currentCapturesMappings().at(name).clone();
        }
    }
    // Step 2: Try global vars
    if (Object *var = Runtime::getInstance().getVar(name)) {
        if (static_cast<TCVar *>(var->m_Data)->m_IsMacro) {
            throw std::runtime_error(std::string("Cannot take value of a macro: #'").append(name));
        }
        return std::make_unique<VarDerefExpr>(var);
    }
    throw std::runtime_error(std::string("Cannot resolve symbol: ").append(name).append(" in the context"));
}

AExpr SemanticAnalyzer::analyze(AnalyzerContext &ctx, const Object *form) {
    return analyze(ExpressionMode::EXPR, ctx, form);
}

Object *SemanticAnalyzer::macroexpand1(Runtime &rt, const Object *form) {
    if (form == nullptr || form->m_Type != ObjectType::LIST) {
        return const_cast<Object *>(form);
    }
    const Object *head = tc_list_first(form);
    if (head == nullptr || head->m_Type != ObjectType::SYMBOL) {
        return const_cast<Object *>(form);
    }
    const std::string &sym = tc_symbol_valueX(head);
    if (Object *var = rt.getVar(sym)) {
        if (static_cast<TCVar *>(var->m_Data)->m_IsMacro) {
            const Object *arglist = tc_list_next(form);
            tc_int_t argc = static_cast<TCInteger *>(tc_list_length(arglist)->m_Data)->m_Value;
            const Object **argv = new const Object *[argc];
            for (size_t i = 0; i < static_cast<size_t>(argc); i++) {
                argv[i] = const_cast<Object *>(tc_list_first(arglist));
                arglist = tc_list_next(arglist);
            }
            Object *expanded = (var->m_Call)(var, argc, argv);
            delete[] argv;
            return expanded;
        } else {
            return const_cast<Object *>(form);
        }
    } else {
        return const_cast<Object *>(form);
    }
}

Object *SemanticAnalyzer::macroexpand(Runtime &rt, const Object *form) {
    Object *new_form;
    do {
        new_form = macroexpand1(rt, form);
        if (new_form == form) {
            break;
        }
        form = new_form;
    } while (true);
    return const_cast<Object *>(form);
}

AExpr SemanticAnalyzer::analyze(ExpressionMode mode, AnalyzerContext &ctx, const Object *form) {
    form = macroexpand(Runtime::getInstance(), form);
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
                return std::make_unique<QuotedExpr>(form);
            }
            const Object *head = tc_list_first(form);

            if (head == nullptr) {
                throw std::runtime_error("Cannot call nil.");
            }

            if (head->m_Type == ObjectType::SYMBOL) {
                if (auto ana_it = m_SpecialAnalyzers.find(tc_symbol_valueX(head)); ana_it != m_SpecialAnalyzers.end()) {
                    return ana_it->second(mode, ctx, form);
                }
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
        case ObjectType::VAR:
            throw std::runtime_error("Vars are not supported yet");
        default:
            throw std::runtime_error("Unknown object type: " + std::to_string(static_cast<int>(form->m_Type)));
    }
    std::unreachable();
}
