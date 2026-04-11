#include <iostream>

#include "MemoryManager.h"
#include "Runtime.h"
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
#include "GCFrame.h"

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

void MemoryManager::markRoots(Runtime *rt) {
    for (auto &[name, var]: rt->getGlobalVarStorage()) {
        mark(var);
    }

    // protect the current execution frames from collection
    for (GCRootFrame *frame = rt->m_RootStack; frame; frame = frame->m_Prev) {
        for (auto root: frame->m_Roots) {
            mark(root);
        }
    }
}

void MemoryManager::sweep() {
    size_t newSize = 0;
    size_t i = 0;
    for (auto obj: m_Storage) {
        if (obj->m_Marked) {
            // keep the marked object - still reachable
            obj->m_Marked = false;
            m_Storage[newSize++] = obj;
        } else {
            destroyObject(obj);
        }
        i++;
    }
    std::cout << "GC cycle completed. Collected "
              << (m_Storage.size() - newSize)
              << " / " << m_Storage.size() << " objects." << std::endl;
    m_Storage.resize(newSize);
}

void MemoryManager::collectGarbage(Runtime *rt) {
    if (m_GCInProgress) {
        printf("Debug: GC already in progress, skipping this cycle.\n");
        fflush(stdout);
        return;
    }
    m_GCInProgress = true;

    markRoots(rt);
    sweep();
    m_GCInProgress = false;
    m_HeapCapacity = m_Storage.size() * 2; // double the capacity for the next cycle to reduce frequency of GC
}

void MemoryManager::collectGarbageIfNeeded(Runtime *rt) {
    if (m_Storage.size() >= m_HeapCapacity) {
        //collectGarbage(rt);
        std::cerr << "GC is currently disabled." << std::endl;
        m_HeapCapacity *= 2; // double the capacity to avoid frequent GC cycles during testing
    }
}

void MemoryManager::mark(const Object *obj) {
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
            mark(list->m_Head);
            mark(list->m_Tail);
            break;
        }
        case ObjectType::VAR: {
            auto *var = static_cast<TCVar *>(obj->m_Data);
            mark(var->m_Root);
            break;
        }

        case ObjectType::CLOSURE: {
            auto *closure = static_cast<TCClosure *>(obj->m_Data);
            for (size_t i = 0; i < closure->m_NumCaptures; i++) {
                mark(closure->m_Env[i]);
            }
            break;
        }
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
            // todo: it's pretty sad, this has been created using strdup, so we have to use free herex
            // probably should switch to an implemenation with delete[] and avoid strdup altogether
            free(str->m_Value);
            delete str; // delete TCString struct
            break;
        }
        case ObjectType::SYMBOL: {
            TCSymbol *sym = static_cast<TCSymbol *>(obj->m_Data);
            // todo: same issue as with TCString - should switch to delete[] and avoid strdup
            free(sym->m_Name);
            delete sym;
            break;
        }
        case ObjectType::FUNCTION:
            delete static_cast<TCFunction *>(obj->m_Data);
            break;
        case ObjectType::LIST:
            // do NOT destroy the list recursively - only destroy what has been put in the mark
            // list!!!!!
            delete static_cast<TCList *>(obj->m_Data);
            break;
        case ObjectType::VAR:
            // same as list
            delete static_cast<TCVar *>(obj->m_Data);
            break;
        case ObjectType::CLOSURE: {
            TCClosure *closure = static_cast<TCClosure *>(obj->m_Data);
            // same as list and var - only destroy the closure struct, not the captured environment
            // (which should have been marked and will be collected separately if unreachable)
            delete[] closure->m_Env;
            delete closure;
        }
    }
    delete obj;
}
