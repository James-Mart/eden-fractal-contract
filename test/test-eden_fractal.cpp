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
        auto self = t.as(eden_fractal::default_contract_account);

        THEN("Alice cannot call the setagreement action")
        {
            auto trace = alice.trace<actions::setagreement>("test");
            CHECK(failedWith(trace, missingRequiredAuth));
        }

        THEN("Self can call the setagreement action")
        {
            auto trace = self.trace<actions::setagreement>("test");
            CHECK(succeeded(trace));
        }

        WHEN("Self calls the setagreement action")
        {
            std::string agreementStr = "test";
            self.act<actions::setagreement>(agreementStr);

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
        auto self = t.as(eden_fractal::default_contract_account);

        THEN("Alice cannot sign the agreement before it's added")
        {
            auto trace = alice.trace<actions::sign>("alice"_n);
            CHECK(failedWith(trace, noAgreement));
        }
        WHEN("An agreement is added")
        {
            self.act<actions::setagreement>("test");

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

SCENARIO("Rank submission")
{
    GIVEN("Standard setup, and an admin has a ranking to submit")
    {
        // This starts a single-producer chain
        test_chain t;
        setup_installMyContract(t);
        setup_createAccounts(t);
        setup_token(t);

        auto alice = t.as("alice"_n);
        auto self = t.as(eden_fractal::default_contract_account);

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

        THEN("Self may submit a ranking")
        {
            auto submitRanks = self.trace<actions::submitranks>(util::from_json<AllRankings>(ranks));
            CHECK(succeeded(submitRanks));
        }
        THEN("A non-admin may not submit a ranking")
        {
            auto submitRanks = alice.trace<actions::submitranks>(util::from_json<AllRankings>(ranks));
            CHECK(failedWith(submitRanks, missingRequiredAuth));
        }

        THEN("A ranking with too few groups cannot be submitted")
        {
            AllRankings ar{{{{"james"_n, "dan"_n, "alice"_n, "bob"_n, "charlie"_n, "igor"_n}}}};
            auto submitRanks = self.trace<actions::submitranks>(ar);
            CHECK(failedWith(submitRanks, too_few_groups));
        }
        THEN("A ranking with a group of too few members cannot be submitted")
        {
            AllRankings ar{{{{"james"_n, "dan"_n, "alice"_n, "bob"_n}}, {{"david"_n, "elaine"_n, "frank"_n, "gary"_n, "harry"_n, "jenny"_n}}}};
            auto submitRanks = self.trace<actions::submitranks>(ar);
            CHECK(failedWith(submitRanks, group_too_small));
        }
        THEN("A ranking with a group of too many memmbers cannot be submitted")
        {
            AllRankings ar{{{{"james"_n, "dan"_n, "alice"_n, "bob"_n, "charlie"_n, "igor"_n, "kathy"_n}}, {{"david"_n, "elaine"_n, "frank"_n, "gary"_n, "harry"_n, "jenny"_n}}}};
            auto submitRanks = self.trace<actions::submitranks>(ar);
            CHECK(failedWith(submitRanks, group_too_large));
        }
        THEN("A ranking with two groups that share a person cannot be submitted")
        {
            AllRankings ar{{{{"james"_n, "dan"_n, "alice"_n, "bob"_n, "charlie"_n, "igor"_n}}, {{"james"_n, "elaine"_n, "frank"_n, "gary"_n, "harry"_n, "jenny"_n}}}};
            auto submitRanks = self.trace<actions::submitranks>(ar);
            CHECK(failed(submitRanks));  // Error message is dynamic
        }
    }
}

SCENARIO("Reward distribution")
{
    GIVEN("Standard setup, and an admin has a ranking to submit")
    {
        // This starts a single-producer chain
        test_chain t;
        setup_installMyContract(t);
        setup_createAccounts(t);
        setup_token(t);

        auto self = t.as(eden_fractal::default_contract_account);

        // Give the eden fractal some EOS
        t.as("eosio"_n).act<token::actions::issue>("eosio"_n, s2a("1000000.0000 EOS"), "");
        t.as("eosio"_n).act<token::actions::transfer>("eosio"_n, default_contract_account, s2a("10000.0000 EOS"), "");

        AllRankings ranks{{{{"james"_n, "dan"_n, "alice"_n, "bob"_n, "charlie"_n, "igor"_n}}, {{"david"_n, "elaine"_n, "frank"_n, "gary"_n, "harry"_n, "jenny"_n}}}};

        auto getJamesEden = []() {
            fractal_contract::accounts accountstable(eden_fractal::default_contract_account, "james"_n.value);
            auto itr = accountstable.find(eden_fractal::eden_symbol.code().raw());
            if (itr == accountstable.end()) {
                return (int64_t)0;
            }
            else {
                return itr->balance.amount;
            }
        };

        auto getJamesEOS = []() {
            token::contract::accounts accountstable("eosio.token"_n, "james"_n.value);
            auto itr = accountstable.find(eden_fractal::eos_symbol.code().raw());
            if (itr == accountstable.end()) {
                return (int64_t)0;
            }
            else {
                return itr->balance.amount;
            }
        };

        THEN("James has 0 Eden and 0 EOS")
        {  //
            CHECK(getJamesEden() == 0);
            CHECK(getJamesEOS() == 0);
        }
        WHEN("The ranking is submitted and James is rank 1")
        {
            self.act<actions::submitranks>(ranks);
            THEN("James has 5 EDEN")
            {
                CHECK(getJamesEden() == s2a("5.0000 EDEN").amount);

                AND_THEN("James has 1.8238 EOS")
                {  //
                    CHECK(getJamesEOS() == s2a("1.8238 EOS").amount);
                }
            }
        }

        /*
        void eosrewardamt(const asset& quantity);
        void fiboffset(uint8_t offset);
        */

        WHEN("EOS reward changes to 200")
        {
            auto changeReward = self.trace<actions::eosrewardamt>(s2a("200.0000 EOS"));
            CHECK(succeeded(changeReward));

            AND_WHEN("The ranking is submitted and James is rank 1")
            {
                self.act<actions::submitranks>(ranks);
                THEN("James has 5 EDEN")
                {  //
                    CHECK(getJamesEden() == s2a("5.0000 EDEN").amount);

                    AND_THEN("James has 3.6477 EOS")
                    {  //
                        CHECK(getJamesEOS() == s2a("3.6477 EOS").amount);
                    }
                }
            }
        }
        WHEN("The fib offset is changed to 6")
        {
            auto setFibOffset = self.trace<actions::fiboffset>(6);
            CHECK(succeeded(setFibOffset));
            AND_WHEN("The ranking is submitted and James is rank 1")
            {
                self.act<actions::submitranks>(ranks);

                THEN("James has 8 EDEN")
                {
                    CHECK(getJamesEden() == s2a("8.0000 EDEN").amount);

                    AND_THEN("James has 1.8238 EOS")
                    {  //
                        CHECK(getJamesEOS() == s2a("1.8238 EOS").amount);
                    }
                }
            }
        }
    }
}

SCENARIO("Setting reward and fib offset")
{
    GIVEN("Standard setup")
    {
        // This starts a single-producer chain
        test_chain t;
        setup_installMyContract(t);
        setup_createAccounts(t);
        setup_token(t);

        auto alice = t.as("alice"_n);
        auto oldAdmin = t.as("dan"_n);

        THEN("Alice cannot change the fib offset")
        {
            auto trace = alice.trace<actions::fiboffset>(8);
            CHECK(failedWith(trace, missingRequiredAuth));
        }
        THEN("Alice cannot change the EOS reward amount")
        {
            auto trace = alice.trace<actions::eosrewardamt>(s2a("2000.0000 EOS"));
            CHECK(failedWith(trace, missingRequiredAuth));
        }
        THEN("Initial admin accounts cannot change the fib offset or EOS reward amount anymore")
        {
            auto trace = oldAdmin.trace<actions::fiboffset>(8);
            CHECK(failedWith(trace, missingRequiredAuth));
            trace = oldAdmin.trace<actions::eosrewardamt>(s2a("2000.0000 EOS"));
            CHECK(failedWith(trace, missingRequiredAuth));
        }
    }
}

// The rules for these circles work slightly differently than in fractally,
// The team lead cannot be changed.
// There is no concept of full-time or part-time members. Teams can subjectively manage their membership to only include active members.
SCENARIO("Creating and managing a circle")  // (team)
{
    GIVEN("Standard setup")
    {
        test_chain t;
        setup_installMyContract(t);
        setup_createAccounts(t);
        setup_token(t);

        const auto c = "mycircle"_n;
        t.create_account(c);
        auto circle = t.as(c);

        THEN("A team account can create a circle")
        {
            AND_THEN("The circle exists") {}
            AND_THEN("No one can create a circle with the same name") {}
            AND_THEN("The team account cannot create another circle") {}
        }
        WHEN("A circle is created")
        {
            THEN("The circle account can invite members to it")
            {
                AND_THEN("The new member is invited") {}
                AND_THEN("The new member is not on the team") {}
            }
            THEN("Outsiders cannot invite themselves to it") {}
            AND_WHEN("A new member is invited")
            {
                THEN("The invite may be declined by the inviter") {}
                THEN("Someone else cannot accept or reject the invite on behalf of the invitee") {}
                THEN("The new member can accept the invite")
                {
                    AND_THEN("The new member is on the team") {}
                    AND_THEN("The new member is no longer invited") {}
                    AND_THEN("The new member cannot accept the invite again") {}
                }
                THEN("The new member can reject the invite")
                {
                    AND_THEN("The new member is not on the team") {}
                    AND_THEN("The new member is no longer invited") {}
                    AND_THEN("The new member cannot reject the invite again") {}
                }
                AND_WHEN("The new member joins the team")
                {
                    THEN("They may be kicked out of the team by the team lead")
                    {
                        AND_THEN("They are no longer on the team") {}
                        AND_THEN("They may not leave the team again") {}
                    }
                    THEN("They may leave the team")
                    {
                        AND_THEN("They are no longer on the team") {}
                        AND_THEN("They may not leave the team again") {}
                    }
                    THEN("They may be invited to other teams") {}
                    AND_WHEN("They are invited to other teams")
                    {
                        THEN("They may not join the other team") {}
                        AND_WHEN("The member leaves their existing team")
                        {
                            THEN("They may not join the new team until the time restriction has passed") {}
                            WHEN("The time restriction has passed")
                            {
                                THEN("They may join the new team") {}
                            }
                        }
                    }
                }
                AND_WHEN("The circle gets a sum of 12 members + invites")
                {
                    THEN("No more members can be invited") {}
                }
            }
        }
    }
}

SCENARIO("Payouts with cirlces")
{
    GIVEN("Standard setup with a particular ranking to submit")
    {
        // Use the (global) expected distribution for a given ranking

        THEN("A ranking may not be submitted that includes a circle account") {}
        THEN("A ranking may be submitted that includes a circle member") {}
        AND_GIVEN("A circle that has fewer than the minimum number of members")  // == 4
        {
            WHEN("A ranking is submitted that includes a member")
            {
                THEN("The distribution is equivalent to the distribution without any circle members") {}
            }
        }
        AND_GIVEN("A circle that has at least the minimum number of members")
        {
            WHEN("A ranking is submitted that includes a member")
            {
                THEN("All members get their same expected distribution") {}
                THEN("The circle account also gets a distribution that matches the sum of what each circle member earned") {}
                THEN("The circle account does not get any EOS") {}
            }
        }
    }
}