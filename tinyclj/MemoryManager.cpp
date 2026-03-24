#include "MemoryManager.h"
#include "types/TCBoolean.h"
#include "types/TCChar.h"
#include "types/TCClosure.h"
#include "types/TCDouble.h"
#include "types/TCFunction.h"
#include "types/TCInteger.h"
#include "types/TCList.h"
#include "types/TCString.h"
#include "types/TCSymbol.h"
#include "types/TCVar.h"

Object *MemoryManager::createObject(ObjectType type, void *data, CallFn callFn, bool isStatic) {
    Object *obj = new Object{
            .m_Data = data,
            .m_Type = type,
            .m_Call = callFn,
            .m_Marked = false,
            .m_Static = isStatic
    };
    m_Storage.emplace_back(obj);
    return obj;
}

void MemoryManager::mark(Object *obj) {
    if (obj == nullptr || obj->m_Static || obj->m_Marked) {
        return;
    }
    obj->m_Marked = true;

    switch (obj->m_Type) {
        case ObjectType::BOOLEAN:
        case ObjectType::INTEGER:
        case ObjectType::DOUBLE:
        case ObjectType::CHARACTER:
        case ObjectType::STRING:
        case ObjectType::SYMBOL:
        case ObjectType::FUNCTION:
            break; // no references to other objects, so nothing to mark
        case ObjectType::LIST: {
            auto *list = static_cast<TCList *>(obj->m_Data);
            mark(const_cast<Object *>(list->m_Head));
            mark(const_cast<Object *>(list->m_Tail));
        }
        case ObjectType::VAR: {
            auto *var = static_cast<TCVar *>(obj->m_Data);
            mark(const_cast<Object *>(var->m_Root));
        }

        case ObjectType::CLOSURE:
            auto *closure = static_cast<TCClosure *>(obj->m_Data);
            for (size_t i = 0; i < closure->m_NumCaptures; i++) {
                mark(const_cast<Object *>(closure->m_Env[i]));
            }
            break;
    }
}

void MemoryManager::destroyObject(Object *obj) {
    if (obj == nullptr || obj->m_Static) {
        return;
    }
    switch (obj->m_Type) {
        case ObjectType::BOOLEAN:
            delete static_cast<TCBoolean *>(obj->m_Data);
            break;
        case ObjectType::INTEGER:
            delete static_cast<TCInteger *>(obj->m_Data);
            break;
        case ObjectType::DOUBLE:
            delete static_cast<TCDouble *>(obj->m_Data);
            break;
        case ObjectType::CHARACTER:
            delete static_cast<TCChar *>(obj->m_Data);
            break;
        case ObjectType::STRING: {
            TCString *str = static_cast<TCString *>(obj->m_Data);
            delete[] str->m_Value; // delete char array
            delete str; // delete TCString struct
            break;
        }
        case ObjectType::SYMBOL: {
            TCSymbol *sym = static_cast<TCSymbol *>(obj->m_Data);
            delete[] sym->m_Name;
            delete sym;
            break;
        }
        case ObjectType::FUNCTION:
            delete static_cast<TCFunction *>(obj->m_Data);
            break;
        case ObjectType::LIST: {
            TCList *list = static_cast<TCList *>(obj->m_Data);
            // recursively destroy head and tail
            destroyObject(const_cast<Object *>(list->m_Head));
            destroyObject(const_cast<Object *>(list->m_Tail));
            delete list;
            break;
        }
        case ObjectType::VAR: {
            TCVar *var = static_cast<TCVar *>(obj->m_Data);
            // destroy root
            destroyObject(const_cast<Object *>(var->m_Root));
            delete var;
            break;
        }
        case ObjectType::CLOSURE: {
            TCClosure *closure = static_cast<TCClosure *>(obj->m_Data);
            // destroy captured variables
            for (size_t i = 0; i < closure->m_NumCaptures; i++) {
                destroyObject(const_cast<Object *>(closure->m_Env[i]));
            }
            delete[] closure->m_Env;
            delete closure;
        }
    }
    delete obj;
}
