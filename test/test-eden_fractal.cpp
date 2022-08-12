#include <eosio/tester.hpp>
#include <token/token.hpp>

#include "fractal-contract.hpp"

#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

using namespace eosio;
using namespace eden_fractal;
using namespace eden_fractal::errors;

namespace util {
    template <typename T>
    T from_json(std::string str)
    {
        json_token_stream jsonStream(str.data());
        return from_json<T>(jsonStream);
    }
}  // namespace util

namespace {
    // TODO - Replace with eosio/authority after the new version of clsdk

    struct tester_permission_level_weight {
        eosio::permission_level permission = {};
        uint16_t weight = {};
    };
    EOSIO_REFLECT(tester_permission_level_weight, permission, weight);

    struct tester_wait_weight {
        uint32_t wait_sec = {};
        uint16_t weight = {};
    };
    EOSIO_REFLECT(tester_wait_weight, wait_sec, weight);

    struct tester_authority {
        uint32_t threshold = {};
        std::vector<eosio::key_weight> keys = {};
        std::vector<tester_permission_level_weight> accounts = {};
        std::vector<tester_wait_weight> waits = {};
    };
    EOSIO_REFLECT(tester_authority, threshold, keys, accounts, waits);

    constexpr auto code_permission = "eosio.code"_n;
    permission_level EdenFractalAuth{eden_fractal::default_contract_account, code_permission};

}  // namespace

bool succeeded(const transaction_trace& trace)
{
    if (trace.except) {
        UNSCOPED_INFO("transaction has exception: " << *trace.except << "\n");
    }

    return (trace.status == transaction_status::executed);
}

bool failed(const transaction_trace& trace)
{
    if (trace.except) {
        UNSCOPED_INFO("transaction has exception: " << *trace.except << "\n");
    }

    return (trace.status != transaction_status::executed);
}

bool failedWith(const transaction_trace& trace, std::string_view err)
{
    bool ret = (trace.except->find(err.data()) != string::npos);
    if (!ret && trace.except) {
        UNSCOPED_INFO("transaction has exception: " << *trace.except << "\n");
    }
    return ret;
}

// Set up the token contract
void setup_token(test_chain& t)
{
    t.create_code_account("eosio.token"_n);
    t.set_code("eosio.token"_n, CLSDK_CONTRACTS_DIR "token.wasm");

    // Create and issue tokens.
    t.as("eosio.token"_n).act<token::actions::create>("eosio"_n, s2a("1000000.0000 EOS"));
}

// Setup function to install my contract to the chain
void setup_installMyContract(test_chain& t)
{
    auto contract = eden_fractal::default_contract_account;

    t.create_code_account(contract);
    t.set_code(contract, "artifacts/eden_fractal.wasm");

    t.as(contract).act<actions::create>();
}

// Setup function to add some accounts to the chain
void setup_createAccounts(test_chain& t)
{
    for (auto user : {"alice"_n, "dan"_n, "james"_n, "bob"_n, "charlie"_n, "david"_n, "elaine"_n, "frank"_n, "gary"_n, "harry"_n, "igor"_n, "jenny"_n}) {
        t.create_account(user);
    }
}
