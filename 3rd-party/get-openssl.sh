#!/bin/bash
set -e
set -x

# Argument parsing
if [ -z $1 ]; then
  echo "Give command (install|clean)"
  exit -1
fi
COMMAND=$1
shift 1

# PARAMS of command
while (( "$#" )); do
  case "$1" in
    --version)
      VERSION=$2
      shift 2
      ;;
    --) # end argument parsing
      shift
      break
      ;;
    -*|--*=) # unsupported flags
      echo "Error: Unsupported flag $1" >&2
      exit 1
      ;;
    *) # preserve positional arguments
      PARAMS="$PARAMS $1"
      shift
      ;;
  esac
done

if [ -v ${VERSION} ]; then
  VERSION="1.1.1"
fi

# Transformation
VERSION2="$(echo ${VERSION} | sed 's/\./_/g')"

# functions
install() {
  if [ ! -d openssl ]; then
    git clone https://github.com/openssl/openssl.git
  else
    pushd openssl
    git pull
    popd
  fi

  pushd openssl
    git fetch --all --tags
    git checkout OpenSSL_${VERSION2}-stable
    ./config --prefix=$(pwd)/../../common --openssldir=$(pwd)/../../common no-ssl2 no-ssl3
    make
    make test
    make install
  popd

  echo openssl >> .gitignore
}

clean() {
  rm -rf openssl
}

case ${COMMAND} in
  install)
    install
    ;;
  clean)
    clean
    ;;
  *)
    echo "Unknown command ${COMMAND}, give command (install|clean)"
    exit -1
    ;;    
esac