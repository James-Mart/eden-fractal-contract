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
        constexpr std::string_view missingRequiredAuth = "Missing required authority";

        // Token-related
        constexpr std::string_view tokenAlreadyCreated = "Token already created";
        constexpr std::string_view untradeable = "Token currently untradeable";

        // Ranking related
        constexpr std::string_view requiresEosToken = "Quantity must be denominated in EOS";
        constexpr std::string_view too_few_groups = "Too few groups, at least two groups must be submitted.";
        constexpr std::string_view group_too_small = "One of the groups is too small. Minimum group size = 5.";
        constexpr std::string_view group_too_large = "One of the groups is too large. Maximum group size = 6.";

        // Circle related
        constexpr std::string_view circleBrandTaken = "A circle with this brand was already taken.";
        constexpr std::string_view circleAccountUsed = "Account already used to create a circle.";
        constexpr std::string_view circleAdminRequired = "Circle admin account is required to do that.";
        constexpr std::string_view circleInviteDNE = "Circle invite does not exist.";
        constexpr std::string_view circleDelWithMembers = "Circle cannot be deleted if it still has members.";
        constexpr std::string_view circleNotAMember = "Only a member of that circle can perform that action.";
        constexpr std::string_view circleAlreadyOnTeam = "You cannot join more than one team.";
        constexpr std::string_view circle20DaysJoinDelay = "You must wait a total of 20 days before joining another team.";
        constexpr std::string_view noCircleAccRank = "No circle admin account is allowed to be ranked in consensus rounds. Only circle members.";

    }  // namespace errors
}  // namespace eden_fractal