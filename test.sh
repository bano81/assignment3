#!/bin/bash

# Simple test of the command interpreter

in="abbabaq"
out="0,3,5;"

[[ $(./cmd_int <<< "$in") == "$out"* ]] && echo "PASSED" || echo "FAILED"

in="bbaaccbacq"
out=";"

[[ $(./cmd_int <<< "$in") == "$out"* ]] && echo "PASSED" || echo "FAILED"


in="abbabcacbacq"
out="0;"

[[ $(./cmd_int <<< "$in") == "$out"* ]] && echo "PASSED" || echo "FAILED"