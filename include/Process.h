#pragma once


#include "AliasSKSE.h"



namespace Patch {
    static constexpr inline usize SIZE = 5 * 5;
    static constexpr inline Array<FID, SIZE> SPELL_PERKS_FORMID_ARRAY = {

        /*
            5 * 5: five schools, five perks

            ordering:

            - Novice perks 5,
            - Apprentice perks 5...
        */

    /* NOVICE */
        0x000F2CA6, // Novice ALTER
        0x000F2CA7, // Novice CONJR
        0x000F2CA8, // Novice DESTR
        0x000F2CA9, // Novice ILLUS
        0x000F2CAA, // Novice RESTR

        /* Apprentice */
        0x000C44B7, // ALT
        0x000C44BB, // CONJ
        0x000C44BF, // DESTR
        0x000C44C3, // ILLUS
        0x000C44C7, // RESTR

        /* Adept */
        0x000C44B8,
        0x000C44BC,
        0x000C44C0,
        0x000C44C4,
        0x000C44C8,

        /* EXPERT */
        0x000C44B9,
        0x000C44BD,
        0x000C44C1,
        0x000C44C5,
        0x000C44C9,

        /* MASTER */
        0x000C44BA,
        0x000C44BE,
        0x000C44C2,
        0x000C44C6,
        0x000C44CA
    };
    static_assert ( SPELL_PERKS_FORMID_ARRAY[ SIZE - 1] != 0x0 );


    static inline const usize ARR_LEN = 6;

    /// String: What (Pre/Suf)fix to use , bool: is it a prefix or suffix
    using NameFix = Pair<String, bool>;


    enum SpellLevel {
        kNovice,
        kApprentice,
        kAdept,
        kExpert,
        kMaster,

        kUnknown
    };
    static_assert( static_cast<u8>(SpellLevel::kUnknown) == 6 - 1 );

    static inline Option< Array<Option<NameFix>, ARR_LEN * 2> > ParseTOML();
    static inline Option<SpellLevel> GetSpellPerkLevel(SpellPtr spell);
    static inline Result<NONE, Str> ProcessLoadOrder();


    struct Settings {    
    private:
    /* =============================================== */
        Settings() {}   // default constructor
        
        
        static Settings * INSTANCE_PTR; // this MUST be a pointer


        Array<Option<NameFix>, ARR_LEN> SETTINGS_TOMES = {
            /// Novice, Apprentice ...
            None, None, None, None, None,
            /// none of the above
            None
        };
        

        Array<Option<NameFix>, ARR_LEN> SETTINGS_SPELLS = {
            /// Novice, Apprentice ...
            None, None, None, None, None,
            /// none of the above
            None

        };

    public:
    /* =============================================== */
        Settings(const Settings & settings) = delete;


        static const Settings * GetSingleton() {   // added const to signature to prevent accidental modification later

            /* first time running, calling the parseTOML function */
            if (INSTANCE_PTR == nullptr) {

                auto parseResult = Patch::ParseTOML();
                if (parseResult.has_value()) {

                    /* only initialize if anything is parsed at all */
                    INSTANCE_PTR = new Settings();

                    auto values = parseResult.value();

                    for (usize i = 0; i < ARR_LEN; i++) {
                        INSTANCE_PTR->SETTINGS_TOMES[i] = values.at(i);
                        INSTANCE_PTR->SETTINGS_SPELLS[i] = values.at(i + ARR_LEN);
                    }
                }
            }
            return INSTANCE_PTR;
        }


        static const Array<Option<NameFix>, ARR_LEN> * GetSettings_Spells() {
            return & INSTANCE_PTR->SETTINGS_SPELLS;
        }
        static const Array<Option<NameFix>, ARR_LEN> * GetSettings_Tomes() {
            return & INSTANCE_PTR->SETTINGS_TOMES;
        }


        static bool HasBeenInitialized() {
            return ! (INSTANCE_PTR == nullptr);
        }
    };
};