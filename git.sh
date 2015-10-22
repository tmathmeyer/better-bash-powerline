#!/bin/bash
ZERO="0"

git_module() {
    BRANCH="$(git-branch-name)"
    if [ -n "$BRANCH" ]; then
        echo " $BRANCH$(changes-count)"
    else
        echo ""
    fi
}

git-branch-name() {
    GIT_BRANCH=$(git branch 2>/dev/null | grep "\*" | sed s/*\ //g)
    echo $GIT_BRANCH;
}

changes-count(){
    CHANGES="$(git status --porcelain | wc -l)"
    ZED="0"
    INFO=""
    if [ "$CHANGES" != "$ZED" ]; then
        INFO=":$CHANGES"
    fi
    echo "$INFO"
}

echo $(git_module)
