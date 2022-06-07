#pragma once

#include <eosio/asset.hpp>
#include <eosio/eosio.hpp>
#include <eosio/name.hpp>
#include <eosio/singleton.hpp>
#include <string>
#include <vector>

#include "errors.hpp"
#include "schemas.hpp"

using namespace eosio;
using std::string;
using std::vector;

namespace eden_fractal {

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

    extern const char* eosrewardamt_ricardian;
    extern const char* fiboffset_ricardian;
    extern const char* submitranks_ricardian;

    // The account at which this contract is deployed
    inline constexpr auto default_contract_account = "fractal.eden"_n;

    constexpr std::string_view eden_ticker{"EDEN"};
    constexpr symbol eos_symbol{"EOS", 4};
    constexpr symbol eden_symbol{eden_ticker, 4};

    class fractal_contract : public contract {
       public:
        using eosio::contract::contract;

        using AgreementSingleton = eosio::singleton<"agreement"_n, Agreement>;
        using SignersTable = eosio::multi_index<"signatures"_n, Signature>;
        using accounts = eosio::multi_index<"accounts"_n, account>;
        using stats = eosio::multi_index<"stat"_n, currency_stats>;
        using RewardConfigSingleton = eosio::singleton<"rewardconf"_n, RewardConfig>;

        fractal_contract(name receiver, name code, datastream<const char*> ds);

        // Agreement-related actions
        void setagreement(const std::string& agreement);
        void sign(const name& signer);
        void unsign(const name& signer);

        // Token-related actions
        void create();
        void issue(const name& to, const asset& quantity, const string& memo);
        void retire(const asset& quantity, const string& memo);
        void transfer(const name& from, const name& to, const asset& quantity, const string& memo);
        void open(const name& owner, const symbol& symbol, const name& ram_payer);
        void close(const name& owner, const symbol& symbol);

        // Ranking-related actions (may only be called by admins)
        void eosrewardamt(const asset& quantity);
        void fiboffset(uint8_t offset);
        void submitranks(const AllRankings& ranks);

        // Tester/contract interface to simplify token queries
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

        void validate_quantity(const asset& quantity);
        void validate_memo(const string& memo);
        void validate_symbol(const symbol& symbol);

        void require_admin_auth();
    };

    // clang-format off
    EOSIO_ACTIONS(fractal_contract,
                  default_contract_account,
                  action(setagreement, ricardian_contract(setagreement_ricardian)),
                  action(sign, signer, ricardian_contract(sign_ricardian)),
                  action(unsign, signer, ricardian_contract(unsign_ricardian)),

                  action(create, ricardian_contract(create_ricardian)),
                  action(issue, to, quantity, memo, ricardian_contract(issue_ricardian)),
                  action(retire, quantity, memo, ricardian_contract(retire_ricardian)),
                  action(transfer, from, to, quantity, memo, ricardian_contract(transfer_ricardian)),
                  action(open, owner, symbol, ram_payer, ricardian_contract(open_ricardian)),
                  action(close, owner, symbol, ricardian_contract(close_ricardian)),

                  action(eosrewardamt, quantity, ricardian_contract(eosrewardamt_ricardian)),
                  action(fiboffset, offset, ricardian_contract(fiboffset_ricardian)),
                  action(submitranks, ranks, ricardian_contract(submitranks_ricardian))
                  
    )
    // clang-format on

}  // namespace eden_fractal