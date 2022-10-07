#include "fractal-contract.hpp"

const char* eden_fractal::ricardian_clause = R"(
The Eden Fractal is similar to a fractal as defined in the Fractally whitepaper. <Todo: Include mission statement>
)";

const char* eden_fractal::submitcons_ricardian = R"(
This action enables participants of election to submit rankings of their group members.
)";

const char* eden_fractal::submitcons_ricardian = R"(
This action enables participants of elect delegate into the council.
)";

const char* eden_fractal::startelect_ricardian = R"(
Only callable by an admin. This action increments the election number and sets timer for the election.)";

const char* eden_fractal::setagreement_ricardian = R"(
This action updates the Eden Fractal membership agreement that all community members are required to sign to participate.
)";
const char* eden_fractal::sign_ricardian = R"(
This action indicates that you agree to the mission and rules set forth within the current version of the Eden Fractal membership agreement stored in this contract.
)";
const char* eden_fractal::unsign_ricardian = R"(
This action indicates that you no longer agree to the mission or rules set forth within the current version of the Eden Fractal membership agreement stored in this contract. It will also free any RAM you've allocated to store your signature.
)";

const char* eden_fractal::create_ricardian = R"(
This contract does not allow for the creation of arbitrary assets, it only manages the Eden token.
)";
const char* eden_fractal::issue_ricardian = R"(
Issues (mints) to `to` account a `quantity` of tokens.
)";
const char* eden_fractal::retire_ricardian = R"(
Burns `quantity` tokens.
)";
const char* eden_fractal::transfer_ricardian = R"(
Transfers `quantity` tokens from `from` to `to`.
)";
const char* eden_fractal::open_ricardian = R"(
Allows `ram_payer` to pay to create an account `owner` with zero balance for token `symbol`.
)";
const char* eden_fractal::close_ricardian = R"(
The opposite for open, it closes the account `owner` (balance must be 0).
)";

const char* eden_fractal::eosrewardamt_ricardian = R"(
Only callable by an admin. Sets the total amount of EOS used for distributions after meetings.
)";
const char* eden_fractal::fiboffset_ricardian = R"(
Only callable by an admin. Sets the 0-based index of the fibonacci sequence used for native token distribution to rank 1 
(e.g. if offset = 5, rank 1 members will be allocated 8 new tokens)
)";
const char* eden_fractal::submitranks_ricardian = R"(
Only callable by an admin. Submits all group rankings. Order each group in the order they rank (rank 1 first, rank 6 last).
)";
