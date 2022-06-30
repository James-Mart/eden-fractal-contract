#pragma once

#include <string_view>

namespace eden_fractal {
    namespace errors {

        // Consensus submission-related
        constexpr std::string_view noElections = "No eletions have happened yet.";

        // Agreement-related
        constexpr std::string_view requiresAdmin = "Action requires admin authority. Admins: Dan Singjoy, Joshua Seymour, Chuck Macdonald.";
        constexpr std::string_view alreadySigned = "You already signed the agreement";
        constexpr std::string_view noAgreement = "No agreement has been added yet";
        constexpr std::string_view notSigned = "You haven't signed this agreement. Nothing to unsign";

        // Token-related
        constexpr std::string_view tokenAlreadyCreated = "Token already created";
        constexpr std::string_view untradeable = "Token currently untradeable";

        // Ranking related
        constexpr std::string_view requiresEosToken = "Quantity must be denominated in EOS";
        constexpr std::string_view too_few_groups = "Too few groups, at least two groups must be submitted.";
        constexpr std::string_view group_too_small = "One of the groups is too small. Minimum group size = 5.";
        constexpr std::string_view group_too_large = "One of the groups is too large. Maximum group size = 6.";

    }  // namespace errors
}  // namespace eden_fractal