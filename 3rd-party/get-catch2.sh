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
  VERSION="2.12.1"
fi


# functions
install() {
  if [ ! -d Catch2 ]; then
    git clone https://github.com/catchorg/Catch2.git
  else
    pushd Catch2
    git checkout master
    git branch -d latest-Catch2
    git pull
    popd
  fi

  pushd Catch2
    git fetch --all --tags
    git checkout tags/v${VERSION} -b latest-Catch2
    cmake -DCMAKE_INSTALL_PREFIX=$(pwd)/../../common -Bbuild -H. -DBUILD_TESTING=OFF
    cmake --build build/ --target install
  popd

  echo Catch2 >> .gitignore
}

clean() {
  rm -rf Catch2
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
