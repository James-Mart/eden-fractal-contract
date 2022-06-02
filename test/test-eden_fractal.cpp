#include <eosio/tester.hpp>
#include <token/token.hpp>

#include "fractal-contract.hpp"

#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

using namespace eosio;
using namespace eden_fractal;
using namespace eden_fractal::errors;

bool succeeded(const transaction_trace& trace)
{
    if (trace.status == transaction_status::executed) {
        return true;
    }
    else {
        UNSCOPED_INFO("Was supposed to succeed, but failed with : " + trace.except.value());
        return false;
    }
}

bool failedWith(const transaction_trace& trace, std::string_view err)
{
    if (trace.except->find(err.data()) != std::string::npos) {
        return true;
    }
    else {
        auto intendedErr = std::string(err.data());
        UNSCOPED_INFO("Was supposed to fail with: " + intendedErr + " but it failed with: " + trace.except.value());
        return false;
    }
}

// Setup function to install my contract to the chain
void setup_installMyContract(test_chain& t)
{
    t.create_code_account(eden_fractal::default_contract_account);
    t.set_code(eden_fractal::default_contract_account, "artifacts/eden_fractal.wasm");
}

// Setup function to add some accounts to the chain
void setup_createAccounts(test_chain& t)
{
    for (auto user : {"alice"_n, "dan"_n}) {
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

// Todo
// SCENARIO("Token tests")
// SCENARIO("Reward customization")
// SCENARIO("Rank submission")