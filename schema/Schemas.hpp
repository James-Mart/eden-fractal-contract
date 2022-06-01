#pragma once

#include <eosio/eosio.hpp>
#include <eosio/name.hpp>
#include <string>

namespace eden_fractal
{
    struct Agreement
    {
        std::string agreement;
        uint8_t versionNr;
    };
    EOSIO_REFLECT(Agreement, agreement, versionNr);

    struct Signatures
    {
        eosio::name signer;
        uint64_t primary_key() const {return signer.value;}
    };
    EOSIO_REFLECT(Signatures, signer);
}  // namespace eden_fractal