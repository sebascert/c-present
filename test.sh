#!/usr/bin/env bash

# WARN fully AI generated

# set -euo pipefail   # <-- remove -e so failures don't abort the whole script
set -uo pipefail

hex2bin() { xxd -r -p; }
bin2hex() { xxd -p -c 64 | tr -d '\n'; }

run_enc() {
    local key_hex="$1" pt_hex="$2"
    printf '%s' "$key_hex" | hex2bin > key.bin
    # capture pipeline status without aborting
    local out
    out="$( printf '%s' "$pt_hex" | hex2bin | ./cpresent -e key.bin | bin2hex )" || return 1
    printf '%s' "$out"
}

run_dec() {
    local key_hex="$1" ct_hex="$2"
    printf '%s' "$key_hex" | hex2bin > key.bin
    local out
    out="$( printf '%s' "$ct_hex" | hex2bin | ./cpresent -d key.bin | bin2hex )" || return 1
    printf '%s' "$out"
}

# Example loop
PTS=(0000000000000000 0000000000000000 FFFFFFFFFFFFFFFF FFFFFFFFFFFFFFFF)
KEYS=(00000000000000000000 FFFFFFFFFFFFFFFFFFFF 00000000000000000000 FFFFFFFFFFFFFFFFFFFF)
CTS=(5579C1387B228445 E72C46C0F5945049 A112FFC72F68417B 3333DCD3213210D2)

pass=0 fail=0
echo "== PRESENT-80 Known-Answer Tests =="
for i in "${!PTS[@]}"; do
    got_enc="$(run_enc "${KEYS[$i]}" "${PTS[$i]}")" || got_enc="<error>"
    if [[ "${got_enc^^}" == "${CTS[$i]^^}" ]]; then
        echo "  [E$i] PASS  PT=${PTS[$i]}  CT=$got_enc"
        ((pass++))
    else
        echo "  [E$i] FAIL  PT=${PTS[$i]}  got=$got_enc  exp=${CTS[$i]}"
        ((fail++))
    fi

    got_dec="$(run_dec "${KEYS[$i]}" "${CTS[$i]}")" || got_dec="<error>"
    if [[ "${got_dec^^}" == "${PTS[$i]^^}" ]]; then
        echo "  [D$i] PASS  CT=${CTS[$i]}  PT=$got_dec"
        ((pass++))
    else
        echo "  [D$i] FAIL  CT=${CTS[$i]}  got=$got_dec  exp=${PTS[$i]}"
        ((fail++))
    fi
done

echo "== Summary: PASS=$pass FAIL=$fail =="
# exit non-zero only at the end:
(( fail == 0 ))
