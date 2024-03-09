#pragma once

#include "../src/PCH.h"
#include "Process.h"

namespace ReadConfigs {
    

    using NameFix = Patch::NameFix;
    struct ParsedConfig {
        
        bool ApplyToSpells;
        bool ApplyToTomes;

        NameFix NameAndWhereToPut;

        /* Any match in this is a No Go */
        Vec<KeywordPtr> Kywds_MustNotHave;
        /* Any ONE match in this is a match */
        Vec< Vec<KeywordPtr> > Kydws_CanHave;

        
    public:
        template<typename REFR>
        static inline bool IsAMatch(const ParsedConfig & config, const REFR refr) {

            if (config.NameAndWhereToPut.first.empty()
                | config.objSpell == None
                | config.objSpell == None
            ) {
                return false;
            }

            /* exclude */
            for (const auto & mustNotHave : config.Kywds_MustNotHave) {
                if (refr->HasKeyword(mustNotHave)) {
                    return false;
                }
            }
            /* include */
            for (const auto & canHaveVec : config.Kydws_CanHave) {
                
                bool allMatch = true;
                for (const auto & canHave : canHaveVec) {
                    if ( refr->HasKeyword(canHave) == false ) {
                        allMatch = false;
                        break;
                    }
                }

                if ( allMatch == true ) {
                    return true;
                }                
            }

            return false;
        }

        static inline Option<ParsedConfig> New(
            NameFix NameAndWhereToPut,
            Option<Vec<KeywordPtr>> Kywds_MustNotHave,
            Option<Vec< Vec<KeywordPtr> >> Kydws_CanHave,

            bool ApplyToSpells,
            bool ApplyToTomes
        ) {
            if (
                (Kydws_CanHave == None && Kywds_MustNotHave == None)
                | (ApplyToSpells == false && ApplyToTomes == false)
            ) {
                return None;
            }

            ParsedConfig config;
            config.Kydws_CanHave = Kydws_CanHave.value_or(Vec<KeywordPtr>());
            config.Kywds_MustNotHave = Kydws_CanHave.value_or(Vec<KeywordPtr>());
            
            return config;
        }        

    };



};