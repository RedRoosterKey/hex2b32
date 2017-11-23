#! /bin/bash
set -e
# set -v

# After the first version of hex2b32 was written, base32 was added to coreutils.
# I figured why not use base32, if it is installed, to validate that this program works correctly.

echo Starting base32 parity test

type base32 >/dev/null 2>&1 || { echo >&2 "I require foo but it's not installed.  Aborting."; exit 1; }

DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
PROGRAM="${DIR}/../Release/hex2b32"
RED=$(tput -Txterm setaf 1)
GREEN=$(tput -Txterm setaf 2)
NC=$(tput -Txterm sgr0)

function testOutput() {
    in=${1}
    out=${2}
    options=${3}
    value=$(echo -n "${in}" | ${PROGRAM} ${options})
    if [ "${out}" != "${value}" ]
    then
        echo
        echo "${RED}For input \"${in}\" options \"${options}\""
        echo "Expected \"${out}\""
        echo " but got \"${value}\"${NC}"
        return 255
    else
        echo -n "${GREEN}[OK]${NC} "
    fi
    return 0
}

function testReturnValue() {
    in=${1}
    rVal=${2}
    options=${3}
    set +e
    echo -n "${in}" | ${PROGRAM} ${options} > /dev/null 2>&1
    value=$?
    set -e
    if [ "${rVal}" != "${value}" ]
    then
        echo
        echo "${RED}For input \"${in}\" options \"${options}\""
        echo "Expected \"${rVal}\""
        echo " but got \"${value}\"${NC}"
        return 255
    else
        echo -n "${GREEN}[OK]${NC} "
    fi
    return 0
}

# throw random data at it of various sizes, up to a kilobyte
# verify that hex2b32 gives the sames results as base32
for len in $(seq 1 1024)
do
    raw=$(head -c "${len}" /dev/urandom)
    input=$(echo -n "${raw}" | xxd --cols "${len}" --len "${len}" -ps)
    # expected=$(echo -n "${input}" | xxd --revert -ps | base32 --wrap=0)
    expected=$(echo -n "${raw}" | base32 --wrap=0)
    
    testReturnValue "${input}" 0 ''
    testOutput "${input}" "${expected}" "--input-errors"
done

echo "${GREEN}ALL GOOD!${NC}"

