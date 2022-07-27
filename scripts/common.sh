# This script contains the variables and functions that are common to the other scripts.

# PROJECT_ROOT holds the absolute path to the root directory of the project.
readonly PROJECT_ROOT=$(_() {
    local readonly SCRIPT_DIR=`dirname ${BASH_SOURCE[0]}`
    if [[ ${SCRIPT_DIR} == /* ]]; then
        echo ${SCRIPT_DIR}/..
    else
        echo `pwd`/${SCRIPT_DIR}/..
    fi
}; _)

readonly EXIT_FAILURE=1
readonly EXIT_SUCCESS=0

# fail prints the description of the failure and terminates the script.
fail() {
    echo "Failed to build the project. $*"
    exit ${EXIT_FAILURE}
}

check_build_type() {
    case $1 in
        Debug|Release|RelWithDebInfo)
            ;;
        *)
            echo "Unknown build type: '$1'"
            if [ -z $2 ]; then
                fail
            else
                $2
            fi
            ;;
    esac
}

# lo converts the given string to lower case.
lo() { echo "$*" | tr '[:upper:]' '[:lower:]'; }
# up converts the given string to upper case.
up() { echo "$*" | tr '[:lower:]' '[:upper:]'; }

print_build_title() {
    local readonly MSG="$*"
    local readonly MSG_EXT=$((${#MSG} / 2))
    echo "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%"
    printf "%%%%%*s%*s%%%%\n" $((38+${MSG_EXT})) "${MSG}" $((38-${MSG_EXT})) ""
    echo "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%"
}

any_key() {
    read -rsp $'Press any key to continue...\n' -n1 key
}

# Moves cursor to beginning of next line, $1 lines down.
# If $1 is not given, moves the cursor 1 line down.
next_line() {
    printf "\033[${1:-1}E"
}

#######################################################################################################################
# Check platform-independent prerequisites.

if [ "x$(which cmake)" == "x" ]; then
    fail "Cannot find CMake. Please make sure it is available in your PATH."
fi

if [ "x$(which git)" == "x" ]; then
    fail "Cannot find Git. Please make sure it is available in your PATH."
fi

# code: language='Shell Script' insertSpaces=true tabSize=4