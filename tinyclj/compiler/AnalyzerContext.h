#pragma once

#include <memory>
#include <string>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "llvm/IR/IRBuilder.h"

class FunctionOverload;

class BindingExpr;

class CapturedLocalExpr;

using Captures = std::unordered_map<std::string, CapturedLocalExpr>;

struct AnalyzerContext {
    struct RecurContext {
        size_t m_NumArgs;
        // the loop doesn't actually be referenced from within its body which allows us to emit direct vars
        // instead of phi nodes for the loop variables and additional jump instructions
        bool m_IsReferenced = false;

        RecurContext(size_t numArgs);
    };

    /// A mapping of captured variable name to its index in the closure environment for each stack frame.
    std::vector<Captures> m_CapturesMappingStack;
    /// Whether the current stack frame (function overload) uses captures - the parent function stub
    /// might use captures, but the current function overload might not need the environment struct.
    std::vector<bool> m_CaptureUsedStack;
    /// A mapping of name -> local variable binding for each stack frame.
    std::vector<std::unordered_map<std::string, std::shared_ptr<BindingExpr>>> m_StackFrameBindings;
    /// A set of recur frames.
    std::vector<RecurContext> m_RecurFrames;
    /// A mapping of local variable name to the binding in the current stack frame.
    std::unordered_map<std::string, std::shared_ptr<BindingExpr>> m_ScopeBindings;
    /// A set of module imports for each stack frame.
    std::vector<std::unordered_set<std::string>> m_ModuleImportsStack;

    AnalyzerContext() = default;

    std::unordered_map<std::string, CapturedLocalExpr> &currentCapturesMappings();

    std::unordered_map<std::string, std::shared_ptr<BindingExpr>> &currentStackFrameBindings();

    RecurContext &currentRecurContext();

    unsigned functionDepth() const;
};