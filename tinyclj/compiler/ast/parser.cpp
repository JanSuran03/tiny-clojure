#include "BooleanExpr.h"
#include "CharExpr.h"
#include "DoubleExpr.h"
#include "IntegerExpr.h"
#include "NilExpr.h"
#include "StringExpr.h"
#include "VarExpr.h"
#include "parser.h"
#include "../../types/TCBoolean.h"
#include "../../types/TCChar.h"
#include "../../types/TCDouble.h"
#include "../../types/TCInteger.h"
#include "../../types/TCString.h"
#include "../../types/TCSymbol.h"

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
        case ObjectType::LIST:
            // todo: macroexpansion, special forms, invokes
            throw std::runtime_error("Lists are not supported yet");
        case ObjectType::CHARACTER:
            return std::make_unique<CharExpr>(tc_char_valueX(form));
        case ObjectType::STRING:
            return std::make_unique<StringExpr>(tc_string_valueX(form));
        case ObjectType::SYMBOL:
            return VarExpr::resolveVar(ctx, tc_symbol_valueX(form));
        case ObjectType::FUNCTION:
            throw std::runtime_error("Functions are not supported yet");
    }
}
