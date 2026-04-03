#pragma once

#include "Object.h"

struct TCVar {
    const Object *m_Root = nullptr;
    char *m_Name;
    bool m_IsMacro = false;

    static llvm::StructType *getVarStructType(CodegenContext &ctx);

    static llvm::Value *emitGetRoot(CodegenContext &ctx, llvm::Value *varObjPtr);
};

extern "C" {
Object *tc_var_new(const char *name);

const Object *tc_var_get_root(Object *var);

void tc_var_bind_root(Object *var, const Object *obj);

bool tc_var_is_macroX(const Object *var);

void tc_var_set_macroX(Object *var, bool is_macro);
}
