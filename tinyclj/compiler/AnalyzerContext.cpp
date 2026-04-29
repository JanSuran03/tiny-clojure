#include "AnalyzerContext.h"

#include "ast/local-binding/CapturedLocalExpr.h"

AnalyzerContext::RecurContext::RecurContext(size_t numArgs)
        : m_NumArgs(numArgs) {}

std::unordered_map<std::string, CapturedLocalExpr> &AnalyzerContext::currentCapturesMappings() {
    return m_CapturesMappingStack.back();
}

std::unordered_map<std::string, std::shared_ptr<BindingExpr>> &AnalyzerContext::currentStackFrameBindings() {
    return m_StackFrameBindings.back();
}

AnalyzerContext::RecurContext &AnalyzerContext::currentRecurContext() {
    return m_RecurFrames.back();
}

unsigned int AnalyzerContext::functionDepth() const {
    return m_StackFrameBindings.size() - 1;
}
