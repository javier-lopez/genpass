#!/bin/sh

rm -rf /tmp/genpass-*.exp
CURRENT_DIR="$(cd "$(dirname "${0}" )" && pwd)"
PATH="${CURRENT_DIR}/../:${PATH}"; export PATH

expect_script='
log_user 0
spawn genpass-static -f key -C1 -c1 1

set timeout 1

expect {
  "Name:" {
  send "1"
expect {
  "Master password:" {
  send "1"
expect {
  "4ui?YtkC[}e[XHCYs.r9Zsd{NC+aDl/US?h9A)58!"
  }}}}

 timeout {
  puts "genpass failed";
  exit 1
 }
}'

printf "%s\\n" "${expect_script}" > /tmp/genpass-name-and-mp-input.exp
expect /tmp/genpass-name-and-mp-input.exp
