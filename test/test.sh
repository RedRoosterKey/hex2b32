# /bin/bash
set -e
# set -v

DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
PROGRAM="${DIR}/../Release/hex2b32"
RED=`tput setaf 1`
GREEN=`tput setaf 2`
NC=`tput sgr0`

function testOutput() {
    in=${1}
    out=${2}
    options=${3}
    value=`echo -n ${in} | ${PROGRAM} ${options}`
    if [ "${out}" != "${value}" ]
    then
        echo "${RED}Expected \"${out}\""
        echo " but got \"${value}\"${NC}"
        return -1
    else
        echo "${GREEN}[OK]${NC}"
    fi
    return 0
}

function testErrorOutput() {
    in=${1}
    err=${2}
    options=${3}
    set +e
    value=$((echo -n ${in} | ${PROGRAM} ${options}) 2>&1)
    set -e
    if [ "${err}" != "${value}" ]
    then
        echo "${RED}Expected error \"${err}\""
        echo " but got       \"${value}\"${NC}"
        return -1
    else
        echo "${GREEN}[OK]${NC}"
    fi
    return 0
}

function testReturnValue() {
    in=${1}
    rVal=${2}
    options=${3}
    set +e
    echo -n ${in} | ${PROGRAM} ${options} > /dev/null 2>&1
    value=$?
    set -e
    if [ "${rVal}" != "${value}" ]
    then
        echo "${RED}Expected \"${rVal}\""
        echo " but got \"${value}\"${NC}"
        return -1
    else
        echo "${GREEN}[OK]${NC}"
    fi
    return 0
}

# Test that nothing does nothing
testOutput ''           ''

# Test that program cannot handle just one hex character
testReturnValue 'F' 1
testErrorOutput 'F' 'Must provide an even number of hexadecimal characters.'

# Test help output
testOutput '' "Usage: hex2b32 [OPTION]... 
Inputs hexadecimal data from STDIN and outputs base32 (RFC 3548) to STDOUT

    -e, --input-errors    display first input error and exit with failure
                          (default behavior is to ignore invalid input)
    -h, --help            display this help message and exit
    -l, --lower           output only lower case letters
                          (default behavior is all upper case)
    -n, --no-padding      omit trailing '=' symbols
    -v, --version         output version information and exit" '-h'
testOutput '' "Usage: hex2b32 [OPTION]... 
Inputs hexadecimal data from STDIN and outputs base32 (RFC 3548) to STDOUT

    -e, --input-errors    display first input error and exit with failure
                          (default behavior is to ignore invalid input)
    -h, --help            display this help message and exit
    -l, --lower           output only lower case letters
                          (default behavior is all upper case)
    -n, --no-padding      omit trailing '=' symbols
    -v, --version         output version information and exit" '--help'

# Test ignoring non-hex data
testOutput '-0-0---0-0-'  'AAAA===='

# Test error on input
testReturnValue '-' 1 '-e'
testErrorOutput '-' "Invalid hexadecimal character '-'." '-e'
testReturnValue '-' 1 '--input-errors'
testErrorOutput '-' "Invalid hexadecimal character '-'." '--input-errors'

# Test if padding is removed with short option
testOutput '00'           'AA'         '-n'
testOutput '0000'         'AAAA'       '-n'
testOutput '000000'       'AAAAA'      '-n'
testOutput '00000000'     'AAAAAAA'    '-n'
testOutput '0000000000'   'AAAAAAAA'   '-n'
testOutput '000000000000' 'AAAAAAAAAA' '-n'
testOutput 'FF'           '74'         '-n'
testOutput 'FFFF'         '777Q'       '-n'
testOutput 'FFFFFF'       '77776'      '-n'
testOutput 'FFFFFFFF'     '777777Y'    '-n'
testOutput 'FFFFFFFFFF'   '77777777'   '-n'
testOutput 'FFFFFFFFFFFF' '7777777774' '-n'

# Test lowercase with short option
testOutput '223EBB2FF18F01B4605F9CFEEA9D062FDDCC2CEAFF0BA66954929A93BE552F54' 'ei7lwl7rr4a3iyc7tt7ovhigf7o4ylhk74f2m2kusknjhpsvf5ka====' '-l'
# Test lowercase with long option
testOutput '223EBB2FF18F01B4605F9CFEEA9D062FDDCC2CEAFF0BA66954929A93BE552F54' 'ei7lwl7rr4a3iyc7tt7ovhigf7o4ylhk74f2m2kusknjhpsvf5ka====' '--lower'

# Test if padding is removed with long option
testOutput '00'           'AA'         '--no-padding'
testOutput '0000'         'AAAA'       '--no-padding'
testOutput '000000'       'AAAAA'      '--no-padding'
testOutput '00000000'     'AAAAAAA'    '--no-padding'
testOutput '0000000000'   'AAAAAAAA'   '--no-padding'
testOutput '000000000000' 'AAAAAAAAAA' '--no-padding'
testOutput 'FF'           '74'         '--no-padding'
testOutput 'FFFF'         '777Q'       '--no-padding'
testOutput 'FFFFFF'       '77776'      '--no-padding'
testOutput 'FFFFFFFF'     '777777Y'    '--no-padding'
testOutput 'FFFFFFFFFF'   '77777777'   '--no-padding'
testOutput 'FFFFFFFFFFFF' '7777777774' '--no-padding'

# Test when no padding is required
testOutput '0000000000'   'AAAAAAAA'
testOutput '0000000001'   'AAAAAAAB'
testOutput '0000000002'   'AAAAAAAC'
testOutput '0000000003'   'AAAAAAAD'
testOutput '0000000004'   'AAAAAAAE'
testOutput '0000000005'   'AAAAAAAF'
testOutput '0000000006'   'AAAAAAAG'
testOutput '0000000007'   'AAAAAAAH'
testOutput '0000000008'   'AAAAAAAI'
testOutput '0000000009'   'AAAAAAAJ'
testOutput '000000000A'   'AAAAAAAK'
testOutput '000000000B'   'AAAAAAAL'
testOutput '000000000C'   'AAAAAAAM'
testOutput '000000000D'   'AAAAAAAN'
testOutput '000000000E'   'AAAAAAAO'
testOutput '000000000F'   'AAAAAAAP'
testOutput '0000000010'   'AAAAAAAQ'
testOutput '0000000011'   'AAAAAAAR'
testOutput '0000000012'   'AAAAAAAS'
testOutput '0000000013'   'AAAAAAAT'
testOutput '0000000014'   'AAAAAAAU'
testOutput '0000000015'   'AAAAAAAV'
testOutput '0000000016'   'AAAAAAAW'
testOutput '0000000017'   'AAAAAAAX'
testOutput '0000000018'   'AAAAAAAY'
testOutput '0000000019'   'AAAAAAAZ'
testOutput '000000001A'   'AAAAAAA2'
testOutput '000000001B'   'AAAAAAA3'
testOutput '000000001C'   'AAAAAAA4'
testOutput '000000001D'   'AAAAAAA5'
testOutput '000000001E'   'AAAAAAA6'
testOutput '000000001F'   'AAAAAAA7'
testOutput '0000000020'   'AAAAAABA' # Carry

# Test padding with 3 bits remaining
testOutput '00'           'AA======'
testOutput '01'           'AE======'
testOutput '02'           'AI======'
testOutput '03'           'AM======'
testOutput '04'           'AQ======'
testOutput '05'           'AU======'
testOutput '06'           'AY======'
testOutput '07'           'A4======'
testOutput '08'           'BA======' # Carry

# Test padding with 1 bit remaining
testOutput '0000'         'AAAA===='
testOutput '0001'         'AAAQ===='
testOutput '0002'         'AABA====' # Carry

# Test padding with 4 bits remaining
testOutput '000000'       'AAAAA==='
testOutput '000001'       'AAAAC==='
testOutput '000002'       'AAAAE==='
testOutput '000003'       'AAAAG==='
testOutput '000004'       'AAAAI==='
testOutput '000005'       'AAAAK==='
testOutput '000006'       'AAAAM==='
testOutput '000007'       'AAAAO==='
testOutput '000008'       'AAAAQ==='
testOutput '000009'       'AAAAS==='
testOutput '00000A'       'AAAAU==='
testOutput '00000B'       'AAAAW==='
testOutput '00000C'       'AAAAY==='
testOutput '00000D'       'AAAA2==='
testOutput '00000E'       'AAAA4==='
testOutput '00000F'       'AAAA6==='
testOutput '000010'       'AAABA===' # Carry

# Test padding with 2 bits remaining
testOutput '00000000'     'AAAAAAA='
testOutput '00000001'     'AAAAAAI='
testOutput '00000002'     'AAAAAAQ='
testOutput '00000003'     'AAAAAAY='
testOutput '00000004'     'AAAAABA=' # Carry

# Test some random long data
# 32 bytes
testOutput '223EBB2FF18F01B4605F9CFEEA9D062FDDCC2CEAFF0BA66954929A93BE552F54' 'EI7LWL7RR4A3IYC7TT7OVHIGF7O4YLHK74F2M2KUSKNJHPSVF5KA===='
# 256 Bytes
testOutput '65ccfb9d6668b4ebbc0c7221373a41dc02003a78e9c50e3e2369104193cc75e350bef8ede8d0e58e34342b148762eaf2cedbd4b31c21c3766d3c0e1be03a4bf68a084ac7beee4bc5e55cf076e62ad8a1d4d5dc5d43b4eb23f68d0a281969cd65255ece754028fff77f836d87880962604884c22d7feffa2ab566ab2fba7c023b436657ab4544dd9d126f4539e99994e616d649a57d3506d32e6913a52c1ca6b3776c56a535169349bed764a51c348b473b34f575307d421624e4e635e42b7fe82031889f7ad78f71c98d440e1ac68710efdb8b1b2682851acee539c614598ab87c562b5b450a43f874a206dbe89ef3dffd580b9f58ad9be8bd57a9b1fe639d56' 'MXGPXHLGNC2OXPAMOIQTOOSB3QBAAOTY5HCQ4PRDNEIEDE6MOXRVBPXY5XUNBZMOGQ2CWFEHMLVPFTW32SZRYIODOZWTYDQ34A5EX5UKBBFMPPXOJPC6KXHQO3TCVWFB2TK5YXKDWTVSH5UNBIUBS2ONMUSV5TTVIAUP7537QNWYPCAJMJQERBGCFV7676RKWVTKWL52PQBDWQ3GK6VUKRG5TUJG6RJZ5GMZJZQW2ZE2K7JVA3JS42ITUUWBZJVTO5WFNJJVC2JUTPWXMSSRYNELI45TJ5LVGB6UEFRE4TTDLZBLP7UCAMMIT55NPD3RZGGUIDQ2Y2DRB363RMNSNAUFDLHOKOOGCRMYVOD4KYVVWRIKIP4HJIQG3PUJ54677VMAXH2YVWN6RPKXVGY74Y45KY======'

echo "${GREEN}ALL GOOD!${NC}"