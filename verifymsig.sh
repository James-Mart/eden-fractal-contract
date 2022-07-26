#!/bin/bash

# Parameter 1 = wasm filename of the code in ./build/artifacts/
# Parameter 2 = proposer
# Parameter 3 = proposal name

# Example Usage:
# ./verifymsig.sh eden_fractal.wasm ironscimitar update

echo ""

echo "Sha256 of code in current build/artifacts/"
echo "------------------------------------------"
sha256sum ./build/artifacts/$1

echo ""
echo "Sha256 of code in msig"
echo "------------------------------------------"
cleos -u https://eos.greymass.com multisig review $2 $3 | jq -r '.transaction.actions[0].data.code' | xxd -r -p | sha256sum

echo ""