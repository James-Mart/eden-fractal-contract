#pragma once

#include <eosio/asset.hpp>
#include <eosio/eosio.hpp>
#include <eosio/name.hpp>
#include <string>

namespace eden_fractal {

    //Consesus submission related
    struct Consenzus {
        std::vector<eosio::name> rankings;
        uint64_t groupNr;
        eosio::name submitter;

        uint64_t primary_key() const { return submitter.value; }

        uint64_t get_secondary_1() const { return groupNr; }
    };
    EOSIO_REFLECT(Consenzus, rankings, groupNr, submitter);
    EOSIO_COMPARE(Consenzus);

    struct ElectionInf {
        uint64_t electionNr;
        eosio::time_point_sec starttime;
    };
    EOSIO_REFLECT(ElectionInf, electionNr, starttime);

    /*

    struct Consensus {
        results results0;
        uint64_t electionNr;

        uint64_t primary_key() const { return electionNr; }
    };
    EOSIO_REFLECT(Consensus, results, electionNr);

    struct results {
        std::vector<eosio::name> rankings;
        eosio::name submitter;
    };
    EOSIO_REFLECT(results, rankings, submitter);

    struct ElectionInf {
        uint8_t electionNr;
        time_point_sec electionstart;
    };
    EOSIO_REFLECT(ElectionNr, electionNr, electionstart);

*/
    // Agreement-related
    struct Agreement {
        std::string agreement;
        uint8_t versionNr;
    };
    EOSIO_REFLECT(Agreement, agreement, versionNr);

    struct Signature {
        eosio::name signer;
        uint64_t primary_key() const { return signer.value; }
    };
    EOSIO_REFLECT(Signature, signer);

    // Token-related
    struct account {
        eosio::asset balance;

        uint64_t primary_key() const { return balance.symbol.code().raw(); }
    };
    EOSIO_REFLECT(account, balance);

    struct currency_stats {
        eosio::asset supply;
        eosio::asset max_supply;
        eosio::name issuer;

        uint64_t primary_key() const { return supply.symbol.code().raw(); }
    };
    EOSIO_REFLECT(currency_stats, supply, max_supply, issuer);

    // Ranking-related
    struct RewardConfig {
        int64_t eos_reward_amt;
        uint8_t fib_offset;
    };
    EOSIO_REFLECT(RewardConfig, eos_reward_amt, fib_offset);

    struct GroupRanking {
        std::vector<eosio::name> ranking;
    };
    EOSIO_REFLECT(GroupRanking, ranking);
    struct AllRankings {
        std::vector<GroupRanking> allRankings;
    };
    EOSIO_REFLECT(AllRankings, allRankings);

}  // namespace eden_fractal