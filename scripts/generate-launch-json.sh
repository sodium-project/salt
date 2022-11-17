#!/usr/bin/env bash
source ${BASH_SOURCE[0]%/*}/common.sh

temporary_file=$(mktemp "${TMPDIR:-/tmp}/launch.json.XXXXXXXX")
function cleanup {
    rm -rf "${temporary_file}"
}
trap cleanup EXIT

# $1 - the application name
# $2 - the application path
# $3 - separator
function make_launch_configuration {
cat <<EOF
$3
            "type":     "lldb",
            "request":  "launch",
            "name":     "Debug $1",
            "program":  "\${workspaceFolder}/$2",
            "args":     [],
            "cwd":      "\${workspaceFolder}/$(dirname $2)"
EOF
}

function collect_launch_configurations {
    pushd ${PROJECT_ROOT} &>/dev/null
    separator="{"
    for folder in ./build/macosx-arm64-opengl-debug/salt-*; do
        for unit_test in ${folder}/*_tests; do
            if [ -x ${unit_test} ]; then
                make_launch_configuration "$(basename ${unit_test})" "${unit_test}" "${separator}"
                separator=$(printf "        }, {")
            fi
        done

        for exec in ${folder}/*.app/Contents/MacOS/*_macosx; do
            if [ -x ${exec} ]; then
                make_launch_configuration "$(basename ${exec})" "${exec}" "${separator}"
                separator=$(printf "        }, {")
            fi
        done
    done
    if [ "${separator}" != "{" ]; then
        echo "        }"
    fi
    popd &>/dev/null
}

cat <<EOF >"${temporary_file}"
{
    //--------------------------------------------------------------------------------
    // This launch.json was auto generated!
    //
    // It requires CodeLLDB extension to be installed in your Visual Studio Code.
    //--------------------------------------------------------------------------------
    "version": "0.2.0",
    "configurations": [
        $(collect_launch_configurations)
    ]
}
EOF

mv "${temporary_file}" "${PROJECT_ROOT}/.vscode/launch.json"
