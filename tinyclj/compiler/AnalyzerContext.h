#pragma once

#include <memory>
#include <string>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "llvm/IR/IRBuilder.h"

#include "CodegenContext.h"

class FunctionOverload;

class BindingExpr;

class CapturedLocalExpr;

using Captures = std::unordered_map<std::string, CapturedLocalExpr>;

struct AnalyzerContext {
    CodegenContext m_CodegenContext;
    /// A mapping of captured variable name to its index in the closure environment for each stack frame.
    std::vector<Captures> m_CapturesMappingStack;
    /// Whether the current stack frame (function overload) uses captures - the parent function stub
    /// might use captures, but the current function overload might not need the environment struct.
    std::vector<bool> m_CaptureUsedStack;
    /// A mapping of name -> local variable binding for each stack frame.
    std::vector<std::unordered_map<std::string, std::shared_ptr<BindingExpr>>> m_StackFrameBindings;
    /// A set of the count of recur arguments for each recur frame.
    std::vector<size_t> m_NumRecurArgsStack;
    /// For each invoke expression in the current function frame, a vector of the argument counts which
    /// need to be reserved on the stack on the caller side for the native call to the callee stub function.
    std::vector<std::vector<unsigned>> m_InvokeArgCountsStack;
    /// A mapping of local variable name to the binding in the current stack frame.
    std::unordered_map<std::string, std::shared_ptr<BindingExpr>> m_ScopeBindings;

    AnalyzerContext() = default;

    std::unordered_map<std::string, CapturedLocalExpr> &currentCapturesMappings();

    std::unordered_map<std::string, std::shared_ptr<BindingExpr>> &currentStackFrameBindings();

    size_t &currentRecurArgCount();

    std::vector<unsigned> &currentInvokeArgCounts();

    unsigned functionDepth() const;
};