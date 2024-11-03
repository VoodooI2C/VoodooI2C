#!/bin/bash
cd "$(dirname $0)/.."

WARNINGS=(
    '-Wnon-modular-include-in-framework-module'
    '-Werror=non-modular-include-in-framework-module'
    '-Wno-missing-field-initializers'
    '-Wno-missing-prototypes'
    '-Werror=return-type'
    '-Wunreachable-code'
    '-Werror=deprecated-objc-isa-usage'
    '-Werror=objc-root-class'
    '-Wno-non-virtual-dtor'
    '-Wno-overloaded-virtual'
    '-Wno-exit-time-destructors'
    '-Wno-missing-braces'
    '-Wparentheses'
    '-Wswitch'
    '-Wunused-function'
    '-Wno-unused-label'
    '-Wno-unused-parameter'
    '-Wno-inconsistent-missing-override'
    '-Wunused-variable'
    '-Wunused-value'
    '-Wempty-body'
    '-Wconditional-uninitialized'
    '-Wno-unknown-pragmas'
    '-Wno-shadow -Wno-four-char-constants'
    '-Wno-conversion'
    '-Wconstant-conversion'
    '-Wint-conversion'
    '-Wbool-conversion'
    '-Wenum-conversion'
    '-Wshorten-64-to-32'
    '-Wno-newline-eof'
    '-Wno-c++11-extensions'
    '-Wdeprecated-declarations'
    '-Winvalid-offsetof'
    '-Wno-sign-conversion'
    '-Winfinite-recursion'
    '-Wno-move'
    '-Wno-inconsistent-missing-override'
    '-Wno-unused-variable'
    '-Wno-trigraphs'
    '-Wno-non-c-typedef-for-linkage'
)

INCLUDES=(
    '-IMacKernelSDK/Headers'
    '-IDependencies/VoodooInput/Debug/VoodooInput.kext/Contents/Resources'
)

ARGS=(
    '-x c++'
    '-arch x86_64'
    '-fdiagnostics-show-note-include-stack'
    '-fmacro-backtrace-limit=0'
    '-nostdinc'
    '-std=gnu++11'
    '-stdlib=libc++'
    '-fmodules'
    '-gmodules'
    '-fno-builtin'
    '-fno-exceptions'
    '-fno-rtti'
    '-msoft-float'
    '-fno-common'
    '-mkernel'
    '-DDEBUG=1'
    '-DKERNEL'
    '-DKERNEL_PRIVATE'
    '-DDRIVER_PRIVATE'
    '-DAPPLE'
    '-DNeXT'
    '-fapple-kext'
    '-fasm-blocks'
    '-fstrict-aliasing'
    '-mmacosx-version-min=10.10'
    "${INCLUDES[@]}"
    "${WARNINGS[@]}"
)

DOC_GEN_FOLDERS=(
    'VoodooI2C/VoodooI2C'
    'Dependencies'
    'Multitouch Support'
    'VoodooI2C Satellites'
)

find "${DOC_GEN_FOLDERS[@]}" -name '*.hpp' -print0 | xargs -0 \
    cldoc generate ${ARGS[@]} -- --output ./docs --report --merge './Documentation' --basedir .
