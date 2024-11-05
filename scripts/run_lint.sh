#!/bin/bash
cd "$(dirname $0)/.."

cpplint --recursive --filter=-build/include_subdir,-build/header_guard,-whitespace/line_length,-runtime/int ./VoodooI2C
