# Intro

This is a smart contract used to facilitate some activities related to the [Eden fractal](https://www.edenfractal.com) on the EOS public blockchain. For the MVP, there is a fixed list of admins who have the ability to submit the admin-only actions on this contract (see "Actions" section for details). The initial admin accounts are: "dan", "jseymour.gm", "chkmacdonald", and "james.vr"

## Actions

Initially, this contract will have the following actions:

### Agreement-related:

* setagreement - This action updates the Eden Fractal membership agreement that all community members are required to sign to participate. Also increments a version number.
* sign - This action indicates that you agree to the mission and rules set forth within the current version of the Eden Fractal membership agreement stored in this contract.
* unsign - This action indicates that you no longer agree to the mission or rules set forth within the current version of the Eden Fractal membership agreement stored in this contract. It will also free any RAM you've allocated to store your signature.

### Token-related:

* create - Takes no parameters, creates the Eden token. This contract does not allow for the creation of arbitrary assets, it only manages the Eden token.
* issue - Issues (mints) to `to` account a `quantity` of tokens.
* retire - Burns `quantity` tokens.
* transfer - Normally transfers `quantity` tokens from `from` to `to`, however the Eden token starts off as untradeable.
* open - Allows `ram_payer` to pay to create an account `owner` with zero balance for token `symbol`.
* close - The opposite of open, it closes the account `owner` (balance must be 0).

### Consensus-meeting-related:

* eosrewardamt - Only callable by an admin. Configures the total amount of EOS used for distributions after meetings.
* fiboffset - Only callable by an admin. Sets the 0-based index of the fibonacci sequence used for native token distribution to rank 1 (e.g. if offset = 5, rank 1 members will be allocated 8 new tokens).
* submitranks - Only callable by an admin. Submits all group rankings. Order each group in the order they rank (rank 1 first, rank 6 last).
* submitcons - Callable by anyone with EOS acc. Action enables each user to submit rankings for members of his group. 
* startelect - Only callable by an admin. Action enables to start new election by incrementing election number and setting time point for the start of the election. 



# Contributing

## How to contribute

To contribute to this smart contract for the [Eden fractal](https://www.edenfractal.com), follow the instructions at the [EOS Power Network - Open EOS Contract](https://github.com/EOSPowerNetwork/vscode-open-eos-contract) repo.
Quick intro video to this tool can be found [here](https://www.youtube.com/watch?v=YZmTEuOdffs).

## Contributing rules

1. Create an issue in github on this repo to describe the change you want to make
2. Request issue feedback in the community discord
3. Make a new branch for your change
4. When ready, pull-request the new branch into the main branch on github
