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
