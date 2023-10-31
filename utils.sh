export OMNIGLASS_PROJECT_ROOT="$(pwd)"
export OMNIGLASS_BUILD_DIR="$OMNIGLASS_PROJECT_ROOT/output"

function cmgen() {
    cmake -S "$OMNIGLASS_PROJECT_ROOT" -B "$OMNIGLASS_BUILD_DIR" &&\
    export OMNIGLASS_BUILDER_WAS_GENERATED=true;
    cd "$OMNIGLASS_PROJECT_ROOT"
}

function rebuild() {
    { [ -v OMNIGLASS_BUILDER_WAS_GENERATED ] && [ -d $OMNIGLASS_BUILD_DIR ] || cmgen; } &&\
    cd "$OMNIGLASS_BUILD_DIR" &&\
    make;
    cd "$OMNIGLASS_PROJECT_ROOT"
}

function debug {
    { [ -v 1 ] && echo "running $1 with lua remote debug" || echo "please provide the executable name as argument"; } && (
    export ZBS=/opt/zbstudio
    export LUA_PATH="./?.lua;$ZBS/lualibs/?/?.lua;$ZBS/lualibs/?.lua"
    export LUA_CPATH="$ZBS/bin/linux/x64/?.so;$ZBS/bin/linux/x64/clibs/?.so"
    cd $OMNIGLASS_BUILD_DIR/bin/
    ./$1
    cd $OMNIGLASS_PROJECT_ROOT;
    )
}
