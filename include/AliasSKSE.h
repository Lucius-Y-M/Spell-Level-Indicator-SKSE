#pragma once



#include "AliasRust.h"



////// for better printing
#define CONSOLE_PRINT RE::ConsoleLog::GetSingleton()->Print



using FID = u32; // RE::FormID = uint32_t


using BoundObjPtr = RE::TESBoundObject *;
using NPCPtr = RE::TESNPC *;
using KeywordPtr = RE::BGSKeyword *;
using SpellPtr = RE::SpellItem *;
using MGEFPtr = RE::ActiveEffect *;
using BookPtr = RE::TESObjectBOOK *;





/*

    REQUIRES PO3_TWEAKS

    e.g. if TPtr = KeywordPtr,
    this returns a Some(keyword: KeywordPtr) OR None
*/
template<typename T>
[[maybe_unused]]
static inline Option<T*> GetByEDID(Str edid) {
    if (T* form = RE::TESForm::LookupByEditorID<T*>(edid); form) {
        return form;
    }

    return None;

}


template<typename T>
[[maybe_unused]]
static inline Option<T*> GetByFormID(FID formID, Str pluginName) {

    // static_assert( IsSame<T*, T*>::value, "Do NOT manually override generic alias T*!" );   // prevent accidental overriding of T* alias

    if (const auto handler = RE::TESDataHandler::GetSingleton(); handler) {
        if (auto * form = handler->LookupForm(formID, pluginName); form) {

            /* note:
                auto x = form->As<Keyword>();

                here, type of x is:

                KeywordPtr (or: RE::BGSKeyword *)
            
            */
            if (T* obj = form->As<T>() ; obj) {
                return obj;
            }
        }
    }
    return None;
}