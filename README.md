# Intro

This is a smart contract used to facilitate some activities related to the [Eden fractal](https://www.edenfractal.com) on the EOS public blockchain. Initially, this contract will have two actions:

* setagreement - Used to store the Eden fractal contributor agreement. Also allows the agreement to be updated and increments a version number.
* sign - Used by community members to sign the agreement, which is an explicit agreement to abide by the mission and rules set forth therein.
* usign - Used by community members to unsign the agreement, which indicates the member no longer agrees to abide by the mission and rules set forth therein. Also frees ram used to store the signature.

# Contributing

To contribute to this smart contract for the [Eden fractal](https://www.edenfractal.com), follow the below instructions.
Quick intro video to this tool can be found [here](https://www.youtube.com/watch?v=YZmTEuOdffs).

## Prerequisites

1. VSCode installed on your development PC
2. [Remote - Containers](https://marketplace.visualstudio.com/items?itemName=ms-vscode-remote.remote-containers) VSCode extension has been installed

## How to use this repo

1. Create a folder on your pc called eden-fractal-contract.
2. Clone this repo into that directory (from within the new directory, it's `git clone <repo_link> .`)
3. Open in VSCode
4. Run the ```Remote-Containers: Rebuild and Reopen in Container``` command in VSCode

VSCode will relaunch, connecting to a new docker container with the EOS contract development environment already configured, and the contract will be ready to work on./

## Misc notes

When VSCode closes, the container stops. The data within the container is not accessible, as it's stored in an unnamed volume mounted on your PC, only accessible through the docker container launched by VSCode.
Any changes you make to the contract will only persist if you commit and push them to github.

## Contributing rules

1. Create an issue in github on this repo to describe the change you want to make
2. Request issue feedback in the community discord
3. Make a new branch for your change
4. When ready, pull-request the new branch into the main branch on github
