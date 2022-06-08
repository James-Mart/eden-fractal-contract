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

SCENARIO("Testing setagreement")
{
    GIVEN("Standard chain setup")
    {
        // This starts a single-producer chain
        test_chain t;
        setup_installMyContract(t);
        setup_createAccounts(t);

        // some shortcuts
        auto alice = t.as("alice"_n);
        auto dan = t.as("dan"_n);

        THEN("Alice cannot call the setagreement action")
        {
            auto trace = alice.trace<actions::setagreement>("test");
            CHECK(failedWith(trace, requiresAdmin));
        }

        THEN("Dan can call the setagreement action")
        {
            auto trace = dan.trace<actions::setagreement>("test");
            CHECK(succeeded(trace));
        }

        WHEN("Dan calls the setagreement action")
        {
            std::string agreementStr = "test";
            dan.act<actions::setagreement>(agreementStr);

            THEN("The agreement matches what he set")
            {
                auto agreementSingleton = fractal_contract::AgreementSingleton(default_contract_account, default_contract_account.value);
                auto agreement = agreementSingleton.get_or_default();
                CHECK(agreement.agreement == agreementStr);
            }
        }
    }
}

SCENARIO("Testing sign and unsign")
{
    GIVEN("Standard chain setup")
    {
        // This starts a single-producer chain
        test_chain t;
        setup_installMyContract(t);
        setup_createAccounts(t);

        // some shortcuts
        auto alice = t.as("alice"_n);
        auto dan = t.as("dan"_n);

        THEN("Alice cannot sign the agreement before it's added")
        {
            auto trace = alice.trace<actions::sign>("alice"_n);
            CHECK(failedWith(trace, noAgreement));
        }
        WHEN("An agreement is added")
        {
            dan.act<actions::setagreement>("test");

            THEN("Alice cannot unsign the agreement before she signs it")
            {
                auto trace = alice.trace<actions::unsign>("alice"_n);
                CHECK(failedWith(trace, notSigned));
            }
            THEN("Alice can sign the agreement")
            {
                auto trace = alice.trace<actions::sign>("alice"_n);
                CHECK(succeeded(trace));
                t.start_block(1000);

                AND_THEN("She cannot sign it again")
                {
                    auto trace2 = alice.trace<actions::sign>("alice"_n);
                    CHECK(failedWith(trace2, alreadySigned));
                }
                AND_THEN("Her signature is stored")
                {
                    auto signerTable = fractal_contract::SignersTable(default_contract_account, default_contract_account.value);
                    bool sigExists = signerTable.find("alice"_n.value) != signerTable.end();
                    CHECK(sigExists);
                }
            }
            WHEN("Alice signs the agreement")
            {
                alice.act<actions::sign>("alice"_n);

                THEN("She can unsign the agreement")
                {
                    auto trace = alice.trace<actions::unsign>("alice"_n);

                    AND_THEN("The signature no longer exists")
                    {
                        auto signerTable = fractal_contract::SignersTable(default_contract_account, default_contract_account.value);
                        bool sigDNE = signerTable.find("alice"_n.value) == signerTable.end();
                        CHECK(sigDNE);
                    }
                }
            }
        }
    }
}

SCENARIO("Testing token transfers")
{
    GIVEN("Standard chain setup")
    {
        // This starts a single-producer chain
        test_chain t;
        setup_installMyContract(t);
        setup_createAccounts(t);

        // some shortcuts
        auto alice = t.as("alice"_n);
        auto dan = t.as("dan"_n);
        auto contract = t.as(eden_fractal::default_contract_account);

        std::string memo = "memo";

        AND_GIVEN("Alice has some Eden tokens")
        {  //
            contract.act<actions::issue>(eden_fractal::default_contract_account, s2a("1000.0000 EDEN"), memo);
            contract.act<actions::transfer>(eden_fractal::default_contract_account, "alice"_n, s2a("500.0000 EDEN"), memo);

            THEN("Alice cannot send the tokens")
            {  //
                auto transfer = alice.trace<actions::transfer>("alice"_n, "bob"_n, s2a("50.0000 EDEN"), memo);
                CHECK(failedWith(transfer, errors::untradeable));
            }
        }
    }
}

/********* The submission JSON should look like this ********
        {
            "allRankings\": [
                {
                    "ranking": [
                        "james",
                        "dan",
                        "alice",
                        "bob",
                        "charlie"
                    ]
                },
                {
                    "ranking": [
                        "david",
                        "elaine",
                        "frank",
                        "gary",
                        "harry"
                    ]
                }
            ]
        };
*/

SCENARIO("Rank submission")
{
    GIVEN("Standard setup, and an admin has a ranking to submit")
    {
        // This starts a single-producer chain
        test_chain t;
        setup_installMyContract(t);
        setup_createAccounts(t);
        setup_token(t);

        auto dan = t.as("dan"_n);

        // clang-format off
        std::string ranks = 
        "{"
            "\"allRankings\": ["
                "{\"ranking\": ["
                        "\"james\","
                        "\"dan\","
                        "\"alice\","
                        "\"bob\","
                        "\"charlie\","
                        "\"igor\""
                    "]},"
                "{\"ranking\": ["
                        "\"david\","
                        "\"elaine\","
                        "\"frank\","
                        "\"gary\","
                        "\"harry\","
                        "\"jenny\""
                    "]}"
            "]"
        "}";
        // clang-format on

        // Give the eden fractal some EOS
        t.as("eosio"_n).act<token::actions::issue>("eosio"_n, s2a("1000000.0000 EOS"), "");
        t.as("eosio"_n).act<token::actions::transfer>("eosio"_n, default_contract_account, s2a("10000.0000 EOS"), "");

        THEN("An admin may submit a ranking")
        {
            auto submitRanks = dan.trace<actions::submitranks>(util::from_json<AllRankings>(ranks));
            CHECK(succeeded(submitRanks));
        }

        /* TODO - Add test cases to confirm all error conditions:
         *   Too few members in a group
         *   Too many members in a group
         *   Too few groups
         *   One name repeated in multiple groups
        */
    }
}

// Could add some tests for:
// SCENARIO("Reward customization")
