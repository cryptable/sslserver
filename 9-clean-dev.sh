#!/bin/bash

pushd 3rd-party
  ./get-openssl.sh clean
# testing framweworks
  ./get-catch2.sh clean
  rm .gitignore
popd

rm -rf common