export OMNIGLASS_PROJECT_ROOT="$(pwd)"
export OMNIGLASS_BUILD_DIR="$OMNIGLASS_PROJECT_ROOT/output"
export USR_PREFIX="/usr"
export CUSTOM_LIB_FOLDER="/usr/lib"
export CUSTOM_CONFIG_FOLDER="/etc"
export CUSTOM_EXEC_FOLDER="/usr/bin"
export CUSTOM_INCLUDE_FOLDER="/usr/include"
function cmgen() {
    {
    cmake \
        -S "$OMNIGLASS_PROJECT_ROOT"\
        -B "$OMNIGLASS_BUILD_DIR"\
        -DCMAKE_INSTALL_PREFIX="$USR_PREFIX"\
        -DCUSTOM_LIB_FOLDER="$CUSTOM_LIB_FOLDER"\
        -DCUSTOM_CONFIG_FOLDER="$CUSTOM_CONFIG_FOLDER"\
        -DCUSTOM_EXEC_FOLDER="$CUSTOM_EXEC_FOLDER"\
        -DCUSTOM_INCLUDE_FOLDER="$CUSTOM_INCLUDE_FOLDER" &&\
    export OMNIGLASS_BUILDER_WAS_GENERATED=true &&\
    cd "$OMNIGLASS_PROJECT_ROOT";
    } || { cd "$OMNIGLASS_PROJECT_ROOT" && return 128; }
}

function rebuild() {
    { [ -v OMNIGLASS_BUILDER_WAS_GENERATED ] && [ -d $OMNIGLASS_BUILD_DIR ] || cmgen; } &&\
    cd "$OMNIGLASS_BUILD_DIR" &&\
    { make && cd "$OMNIGLASS_PROJECT_ROOT"; } || { echo "build failed." && cd "$OMNIGLASS_PROJECT_ROOT" && return 128; }
}

function debug {
    { [ -v 1 ] && echo "running $1 with lua remote debug" || echo "please provide the executable name as argument"; } && (
    export ZBS=/opt/zbstudio &&\
    export LUA_PATH="./?.lua;$ZBS/lualibs/?/?.lua;$ZBS/lualibs/?.lua" &&\
    export LUA_CPATH="$ZBS/bin/linux/x64/?.so;$ZBS/bin/linux/x64/clibs/?.so" &&\
    cd $OMNIGLASS_BUILD_DIR/bin/ &&\
    ./$1 && cd $OMNIGLASS_PROJECT_ROOT;
    ) || { cd $OMNIGLASS_PROJECT_ROOT && return 128; }
}

function reinstall() {
    rebuild &&\
    echo "installing from $OMNIGLASS_BUILD_DIR" &&\
    cmake --install "$OMNIGLASS_BUILD_DIR";
}
