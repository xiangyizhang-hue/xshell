#!/usr/bin/env bash
set -euo pipefail

workdir="$(mktemp -d)"
trap 'rm -rf "$workdir"' EXIT

output="$workdir/output.txt"
printf 'xecho hello world\nxpwd\nquit\n' | ./xshell >"$output"
grep -q 'hello world' "$output"

printf 'xecho redirected > %s\nquit\n' "$workdir/redirected.txt" | ./xshell >/dev/null
grep -q '^redirected$' "$workdir/redirected.txt"

printf 'printf pipeline | tr a-z A-Z\nquit\n' | ./xshell >"$output"
grep -q 'PIPELINE' "$output"

mkdir -p "$workdir/source/nested"
printf 'copy me\n' >"$workdir/source/nested/file.txt"
printf 'xcp -r %s %s\nxrm -r %s\nquit\n' \
  "$workdir/source" "$workdir/copied" "$workdir/copied" | ./xshell >/dev/null
test ! -e "$workdir/copied"

echo 'xshell smoke test: PASS'
