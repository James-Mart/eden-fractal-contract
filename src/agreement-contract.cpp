#include <eosio/eosio.hpp>
#include <eosio/name.hpp>
#include <eosio/action.hpp>
#include <string>
#include <limits>

#include "agreement-contract.hpp"

using namespace eosio;
using std::string;
using namespace eden_fractal;

agreement_contract::agreement_contract(name receiver, name code, datastream<const char*> ds) : contract(receiver, code, ds)
{
    /* NOP */
}

void agreement_contract::setagreement(const std::string& agreement)
{
    check(has_auth("dan"_n) || has_auth("james.vr"_n), setAgreementAuth.data());

    AgreementSingleton singleton(default_contract_account, default_contract_account.value);
    auto record = singleton.get_or_default(Agreement{});
    check(record.versionNr != std::numeric_limits<decltype(record.versionNr)>::max(), "version nr overflow");

    record.agreement = agreement;
    record.versionNr += 1;

    singleton.set(record, get_self());
}

void agreement_contract::sign(const name& signer)
{
    require_auth(signer);

    AgreementSingleton singleton(default_contract_account, default_contract_account.value);
    check(singleton.exists(), noAgreement.data());

    SignersTable table(default_contract_account, default_contract_account.value);

    if (table.find(signer.value) == table.end())
    {
        table.emplace(signer, [&](auto& row){
            row.signer = signer;
        });
    }
    else
    {
        check(false, alreadySigned.data());
    }
}

void agreement_contract::unsign(const name& signer)
{
    require_auth(signer);
    SignersTable table(default_contract_account, default_contract_account.value);

    table.erase(*table.require_find(signer.value, notSigned.data()));
}

EOSIO_ACTION_DISPATCHER(eden_fractal::actions)

// clang-format off
EOSIO_ABIGEN(actions(eden_fractal::actions), 
    table("agreement"_n, eden_fractal::Agreement), 
    table("signatures"_n, eden_fractal::Signatures),
    ricardian_clause("Agreement contracts clause", eden_fractal::ricardian_clause)
)
// clang-format on
