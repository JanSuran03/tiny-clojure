#include "AnalyzerContext.h"

#include "ast/local-binding/CapturedLocalExpr.h"

std::unordered_map<std::string, CapturedLocalExpr> &AnalyzerContext::currentCapturesMappings() {
    return m_CapturesMappingStack.back();
}

std::unordered_map<std::string, std::shared_ptr<LocalBindingExpr>> &AnalyzerContext::currentStackFrameBindings() {
    return m_StackFrameBindings.back();
}

size_t &AnalyzerContext::currentRecurArgCount() {
    return m_NumRecurArgsStack.back();
}

unsigned &AnalyzerContext::currentLocalCount() {
    return m_NumLocalsStack.back();
}

unsigned int AnalyzerContext::functionDepth() const {
    return m_NumLocalsStack.size() - 1;
}
