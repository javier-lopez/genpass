#!/bin/sh
#Based on the GNU shtool test suite

tools_db="./test.db"
tools="$(grep '^@begin' "${tools_db}" 2>/dev/null | sed -e 's/^@begin{//' -e 's/}.*$//')"

for tool in $tools; do
    [ -f "../${tool}" ] || exit 1
done

#override debug string to ensure it works on any posix sh
PS4=">>"

#move to a tmp subdirectory
rm -rf tmp.dir >/dev/null 2>&1
mkdir  tmp.dir || exit 1
cd     tmp.dir || exit 1

failed="0"; passed="0"; ran="0"

printf "%s\\n\\n" "Info - running tests:"

#run tests
for tool in $tools; do
    rm -rf ./* >/dev/null 2>&1
    printf "%s\\n" "${tool} ........................" | awk '{ printf("%s ", substr($0, 0, 25)); }'
    printf "%s\\n" "PATH=../../:../:/bin:/usr/bin" > run.sh
    sed -e "/^@begin{${tool##*/}}/,/^@end/p" -e '1,$d' ../${tools_db} |\
        sed -e '/^@begin/d' -e '/^@end/d' \
        -e 's/\([^\\]\)[ 	]*$/\1 || exit 1/g' >> run.sh
    printf "exit 0\\n" >> run.sh
    sh -x run.sh        > run.log 2>&1

    if [ "${?}" -ne "0" ]; then
        #generate report
        printf "FAILED\\n"
        printf "+---Test------------------------------\\n"
        cat run.sh | sed -e 's/^/| /g'
        printf "+---Trace-----------------------------\\n"
        cat run.log | sed -e 's/^/| /g'
        failed="$((${failed} + 1))"
        printf "+-------------------------------------\\n"
    else
        passed="$((${passed} + 1))"
        printf "ok\\n"
    fi
    ran="$((${ran} + 1))"
done

printf "\\n"

#result and cleanup
if [ "${failed}" -gt "0" ]; then
    printf "FAILED: passed: ${passed}/${ran}, failed: ${failed}/${ran}\\n"
    exit 1
else
    printf "OK: passed: ${passed}/${ran}\\n"
    cd ..
    rm -rf tmp.dir >/dev/null 2>&1
fi

# vim: set ts=8 sw=4 tw=0 ft=sh :
