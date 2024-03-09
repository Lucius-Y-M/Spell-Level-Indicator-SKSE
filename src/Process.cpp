#pragma once

#include "../include/Process.h"



# define TOML_EXCEPTIONS 0    // This allows error handling without using try-catch
#include <toml++/toml.h>
// #include "../build/skyrim/vcpkg_installed/x64-windows-static-md/include/toml++/toml.h"




static constexpr inline Str CONFIG_NAME = "SLAAD_Config.toml";
static constexpr inline Str CONFIG_PATH_AND_NAME = "Data/SKSE/Plugins/SLAAD_Config.toml";
static constexpr inline Str CONFIG_GROUP_NAME_00_GENERAL = "General";


namespace Patch {


    /*
        This stupid thing cost me 5+ hours to figure out:
        when you declare a static member that does NOT have a const value,
        that is JUST a declaration. You must then DEFINE it elsewhere.
        
        Java didn't prepare me for this...
    */
    Settings * Settings::INSTANCE_PTR = nullptr; 




    static inline Option< Array<Option<NameFix>, ARR_LEN * 2> > ParseTOML() {


        static constexpr Array<Str, ARR_LEN * 2> TOME_AND_SPELL_SETTING_NAMES = {
            "TomeName_01_Novice",
            "TomeName_02_Apprentice",
            "TomeName_03_Adept",
            "TomeName_04_Expert",
            "TomeName_05_Master",


            "TomeName_00_Others",


            "SpellName_01_Novice",
            "SpellName_02_Apprentice",
            "SpellName_03_Adept",
            "SpellName_04_Expert",
            "SpellName_05_Master",


            "SpellName_00_Others",


        };
        static_assert( AllOfArrayNotEmpty(TOME_AND_SPELL_SETTING_NAMES) );


    #ifdef TOML_EXCEPTIONS
        #if TOML_EXCEPTIONS == 0
            toml::parse_result result = toml::parse_file(CONFIG_PATH_AND_NAME);

            if ( ! result ) {

                logger::critical("!! FATAL: Config file {} not found. No settings will be loaded.",
                    CONFIG_NAME
                );
                return None;
            }

            auto tbl = std::move(result).table();


            Array<Option<NameFix>, ARR_LEN * 2> parseResult = {
                None, None, None, None, None,
                None,

                None, None, None, None, None,
                None
            };
            for (usize i = 0; i < ARR_LEN * 2; i++) {

                auto setting = tbl[CONFIG_GROUP_NAME_00_GENERAL][TOME_AND_SPELL_SETTING_NAMES[i]];

                if (setting) {

                    Option<String> str = None;
                    if (setting[0].is_value() && setting[0].is_string()) {
                        str = setting[0].as_string()->get();
                    }

                    Option<bool> is_prefix = None;
                    if (setting[1].is_value() && setting[1].is_boolean()) {
                        is_prefix = setting[1].as_boolean()->get();
                    }


                    if (str.has_value() & is_prefix.has_value()) {
                        auto strVal = str.value();
                        auto prefixVal = is_prefix.value();



                        /* skip modifying empty settings (user wants to skip) */
                        if (strVal.empty()) {
                            logger::info(" ~~~~ Skipping settings [{}] due to string being empty.",
                                TOME_AND_SPELL_SETTING_NAMES[i]
                            );

                            continue;
                        }

                        logger::info(" >>>> Setting [{}] reading successful, added string = {}, prefix or suffix? {}",
                            TOME_AND_SPELL_SETTING_NAMES[i],
                            strVal,
                            (prefixVal == true) ? "PREFIX" : "SUFFIX"
                        );

                        auto parsed = Pair<String, bool>(strVal, prefixVal);
                        parseResult[i] = parsed;

                    } else {
                        logger::error("!!!! ERROR: while setting [{}] exists, its format is invalid.",
                            TOME_AND_SPELL_SETTING_NAMES[i]
                        );
                    }


                } else {
                    logger::error("!!!! ERROR: failed to find setting [{}], spells / spell tomes of that level will not be changed.",
                        TOME_AND_SPELL_SETTING_NAMES[i]
                    );
                }
            }
    
            return parseResult;

        #else
            logger::critical("!! FATAL: TOML_EXCEPTIONS is defined but not 0, not compiled.");
            return None;
        #endif
    #else
        logger::critical("!! FATAL: TOML_EXCEPTIONS not defined, not compiled.");
        return None;
    #endif
    }




    static inline Option<SpellLevel> GetSpellPerkLevel(SpellPtr spell) {

        static constexpr usize GROUP_SIZE = 5;

        if (!spell) { return None; }

        if (const auto & perk = spell->data.castingPerk; perk) {

            usize i = 0;
            for (auto ref_formID : SPELL_PERKS_FORMID_ARRAY) {
                
                if (perk->GetFormID() == ref_formID) {
                    if (i <= GROUP_SIZE - 1) {
                        return SpellLevel::kNovice;
                    }
                    else if (i <= 2 * GROUP_SIZE - 1) {
                        return SpellLevel::kApprentice;
                    }
                    else if (i <= 3 * GROUP_SIZE - 1) {
                        return SpellLevel::kAdept;
                    }
                    else if (i <= 4 * GROUP_SIZE - 1) {
                        return SpellLevel::kExpert;
                    }
                    else {
                        return SpellLevel::kMaster;
                    }
                }
                i += 1; 
            }
        }

        return SpellLevel::kUnknown;
    }


    static inline Result<NONE, Str> ProcessLoadOrder() {
        
        const auto HANDLER = RE::TESDataHandler::GetSingleton();
        if ( ! HANDLER ) { return "!! FATAL: Failed to get TESDataHandler"; }

        /*
            This call will also be its initialization    
        */
        const auto * SETTINGS = Settings::GetSingleton();
        if ( Settings::HasBeenInitialized() == false) {
            return "!! Settings have not been initialized; load order not processed.";
        }


        auto books = HANDLER->GetFormArray<RE::TESObjectBOOK>();
        for (auto * book : books) {
            
            if (! book ) { continue; }
            /* current impl of GetSpell() already checks TeachesSpell() */
            
            if (auto spell = book->GetSpell(); spell) {

                auto spelltype = GetSpellPerkLevel(spell).value_or(SpellLevel::kUnknown);
                auto setting_tome = SETTINGS->GetSettings_Tomes()->at(static_cast< usize > (spelltype));
                auto setting_spell = SETTINGS->GetSettings_Spells()->at(static_cast< usize > (spelltype));

                if ( setting_tome.has_value() == false && setting_spell.has_value() == false) {
                    continue;
                }

                if ( setting_tome.has_value() ) {
                    auto & [str, is_prefix] = setting_tome.value();

                    auto tomeOriginalName = String(book->GetFullName());   // this part is fine

                    if (is_prefix) {
                        
                        auto newName = str.append(tomeOriginalName);    // this is not
                        

                        book->SetFullName(newName.c_str());


                    } else {

                        auto newName = tomeOriginalName + String(str);
                        book->SetFullName(newName.c_str());

                    }
                }

                if ( setting_spell.has_value() ) {
                    auto & [str, is_prefix] = setting_spell.value();

                    auto spellOriginalName = String(spell->GetFullName());
                    if (is_prefix) {
                        
                        auto newName = String(str) + spellOriginalName;
                        spell->SetFullName(newName.c_str());

                    } else {

                        auto newName = spellOriginalName + String(str);
                        spell->SetFullName(newName.c_str());

                    }

                }
            }

        }

        return None;
    };
};
