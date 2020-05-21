#!/bin/bash

# Create common directory
mkdir -p common

#get 3rd-party libraries
mkdir -p 3rd-party
pushd ./3rd-party
  touch .gitignore
  ./get-openssl.sh install
# testing framweworks
  ./get-catch2.sh install
popd
