#pragma once

#include <eosio/asset.hpp>
#include <eosio/eosio.hpp>
#include <eosio/name.hpp>
#include <eosio/singleton.hpp>
#include <string>
#include <vector>

#include "errors.hpp"
#include "schemas.hpp"

namespace eden_fractal {
    using eden_fractal::Agreement;
    using eosio::asset;
    using eosio::check;
    using eosio::contract;
    using eosio::datastream;
    using eosio::name;
    using eosio::print;
    using eosio::symbol;
    using eosio::symbol_code;
    using std::string;

    // Ricardian contracts live in ricardian/fractal_contract-ricardian.cpp
    extern const char* ricardian_clause;

    extern const char* setagreement_ricardian;
    extern const char* sign_ricardian;
    extern const char* unsign_ricardian;

    extern const char* create_ricardian;
    extern const char* issue_ricardian;
    extern const char* retire_ricardian;
    extern const char* transfer_ricardian;
    extern const char* open_ricardian;
    extern const char* close_ricardian;

    // The account this contract is normally deployed to
    inline constexpr auto default_contract_account = "fractal.eden"_n;

    class fractal_contract : public contract {
       public:
        using eosio::contract::contract;

        using AgreementSingleton = eosio::singleton<"agreement"_n, Agreement>;
        using SignersTable = eosio::multi_index<"signatures"_n, Signatures>;
        using accounts = eosio::multi_index<"accounts"_n, account>;
        using stats = eosio::multi_index<"stat"_n, currency_stats>;

        fractal_contract(name receiver, name code, datastream<const char*> ds);

        // Agreement-related actions
        void setagreement(const std::string& agreement);
        void sign(const name& signer);
        void unsign(const name& signer);

        // Token-related actions
        void create(const name& issuer, const asset& maximum_supply);
        void issue(const name& to, const asset& quantity, const string& memo);
        void retire(const asset& quantity, const string& memo);
        void transfer(const name& from, const name& to, const asset& quantity, const string& memo);
        void open(const name& owner, const symbol& symbol, const name& ram_payer);
        void close(const name& owner, const symbol& symbol);

        static asset get_supply(const name& token_contract_account, const symbol_code& sym_code)
        {
            stats statstable(token_contract_account, sym_code.raw());
            const auto& st = statstable.get(sym_code.raw());
            return st.supply;
        }

        static asset get_balance(const name& token_contract_account, const name& owner, const symbol_code& sym_code)
        {
            accounts accountstable(token_contract_account, owner.value);
            const auto& ac = accountstable.get(sym_code.raw());
            return ac.balance;
        }

       private:
        void sub_balance(const name& owner, const asset& value);
        void add_balance(const name& owner, const asset& value, const name& ram_payer);
    };

    // clang-format off
    EOSIO_ACTIONS(fractal_contract,
                  default_contract_account,
                  action(setagreement, ricardian_contract(setagreement_ricardian)),
                  action(sign, signer, ricardian_contract(sign_ricardian)),
                  action(unsign, signer, ricardian_contract(unsign_ricardian)),

                  action(create, issuer, maximum_supply, ricardian_contract(create_ricardian)),
                  action(issue, to, quantity, memo, ricardian_contract(issue_ricardian)),
                  action(retire, quantity, memo, ricardian_contract(retire_ricardian)),
                  action(transfer, from, to, quantity, memo, ricardian_contract(transfer_ricardian)),
                  action(open, owner, symbol, ram_payer, ricardian_contract(open_ricardian)),
                  action(close, owner, symbol, ricardian_contract(close_ricardian))
    )
    // clang-format on

}  // namespace eden_fractal