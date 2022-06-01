#pragma once

#include <eosio/asset.hpp>
#include <eosio/eosio.hpp>
#include <eosio/name.hpp>
#include <eosio/singleton.hpp>
#include <string>
#include <vector>

#include "Schemas.hpp"

namespace eden_fractal
{
    using eden_fractal::Agreement;
    using eosio::check;
    using eosio::contract;
    using eosio::datastream;
    using eosio::name;
    using eosio::print;
    using std::string;

    // Ricardian contracts live in ricardian/agreement_contract-ricardian.cpp
    extern const char* setagreement_ricardian;
    extern const char* sign_ricardian;
    extern const char* unsign_ricardian;
    extern const char* ricardian_clause;

    // The account this contract is normally deployed to
    inline constexpr auto default_contract_account = "fractal.eden"_n;

    constexpr std::string_view setAgreementAuth = "Talk to Dan Singjoy or James Mart, they currently control the agreement";
    constexpr std::string_view alreadySigned = "You already signed the agreement";
    constexpr std::string_view noAgreement = "No agreement has been added yet";
    constexpr std::string_view notSigned = "You haven't signed this agreement. Nothing to unsign";

    class agreement_contract : public contract
    {
       public:
        using eosio::contract::contract;

        agreement_contract(name receiver, name code, datastream<const char*> ds);

        void setagreement(const std::string& agreement);

        void sign(const name& signer);

        void unsign(const name& signer);

        using AgreementSingleton = eosio::singleton<"agreement"_n, Agreement>;
        using SignersTable = eosio::multi_index<"signatures"_n, Signatures>;
    };

    // clang-format off
    EOSIO_ACTIONS(agreement_contract,
                  default_contract_account,
                  action(setagreement, ricardian_contract(setagreement_ricardian)),
                  action(sign, signer, ricardian_contract(sign_ricardian)),
                  action(unsign, signer, ricardian_contract(unsign_ricardian))
    )
    // clang-format on

}  // namespace eden_fractal