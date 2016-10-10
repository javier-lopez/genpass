#!/bin/sh
#usage: ./test.sh [tool] [test]
#based on the GNU shtool test suite

_false() { return 1; }

CURRENT_DIR="$(cd "$(dirname "${0}")" && pwd)"
cd "${CURRENT_DIR}"

#override temp debug string, avoid "Bad substitution" errors
PS4=">>"

TMPDIR="test.tmp"
TMPSCRIPT="test.tmp.sh"

#initial score values
failed="0"; passed="0"; ran="0"

#use default or user input ($1)
[ -z "${1}" ] && db_tests="*.db" || db_tests="${1%.db}.db"

for db_test in "${CURRENT_DIR}"/${db_tests}; do
    tool="$(basename "${db_test}")"; tool="${tool%.db}"

    #check tool existence
    if [ -f ../"${tool}" ]; then
        printf "%s\\n\\n" "Info - running '${tool}' tests:"
    else
        printf "%s\\n" "Warning - '${tool}' doesn't exists, skipping ..."
        _false
        continue
    fi

    #read all tests defined in *.db files or user input ($2)
    tests=""; if [ -z "${2}" ]; then
        tests="$(grep '^@begin' "${db_test}" 2>/dev/null | \
                 sed -e 's/^@begin{//' -e 's/}.*$//')"
    else
        tests="${2}"
    fi

    #move to a tmp subdirectory
    rm -rf "${TMPDIR}" || (sleep 1; rm -rf "${TMPDIR}")
    mkdir  "${TMPDIR}" || exit 1
    cd     "${TMPDIR}" || exit 1

    OLDIFS="${IFS}"; IFS='
'   #this a new line
    for test in ${tests}; do
        #check tests exists
        if ! grep "^@begin{$test}" "${db_test}" >/dev/null 2>&1; then
            printf "%s\\n" "Warning - '${tool}' doesn't exists, skipping ..."
            _false
            continue
        fi

        #clean tmp directory
        rm -rf ./* || (sleep 1; rm -rf ./*)
        #print test title
        printf "%s %s\\n" "${test}" "$(printf "%*s" "65" ""|tr ' ' '.')" | \
            awk '{printf("%s ", substr($0, 0, 65));}'

        #generate tmp test script
        printf "%s\\n" "PATH=../..:/bin:/usr/bin" > "${TMPSCRIPT}"
        sed -e "/^@begin{$test}/,/^@end/p" -e '1,$d' "${db_test}" | \
        sed -e '/^@begin/d' -e '/^@end/d' \
            -e 's/\([^\\]\)[        ]*$/\1 || exit 1/g' >> "${TMPSCRIPT}"
        printf "exit 0\\n"   >> "${TMPSCRIPT}"

        #execute current test
        sh -x "${TMPSCRIPT}" >  "${TMPSCRIPT%.sh}.log"  2>&1

        #validate test result
        if [ "${?}" -eq "0" ]; then
            passed="$((${passed} + 1))"
            printf "ok\\n"
        else
            #generate report
            printf "FAILED\\n"
            printf "+---Test------------------------------\\n"
            cat "${TMPSCRIPT}" | sed -e 's/^/| /g'
            printf "+---Trace-----------------------------\\n"
            cat "${TMPSCRIPT%.sh}.log" | sed -e 's/^/| /g'
            failed="$((${failed} + 1))"
            printf "+-------------------------------------\\n"
        fi

        ran="$((${ran} + 1))"
    done

    IFS="${OLDIFS}"
    cd .. && rm -rf "${TMPDIR}" >/dev/null 2>&1
    printf "\\n"
done

#result and cleanup
if [ "${failed}" -gt "0" ]; then
    printf "FAILED: passed: ${passed}/${ran}, failed: ${failed}/${ran}\\n"
    exit 1
else
    printf "OK: passed: ${passed}/${ran}\\n"
fi

# vim: set ts=8 sw=4 tw=0 ft=sh :
