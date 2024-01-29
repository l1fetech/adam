#!/bin/sh

set -eu

export VERSION=${VERSION:-0.0.0}
export GOFLAGS="'-ldflags=-w -s \"-X=github.com/l1fetech/adam/version.Version=$VERSION\" \"-X=github.com/l1fetech/adam/server.mode=release\"'"

docker build \
    --push \
    --platform=linux/arm64,linux/amd64 \
    --build-arg=VERSION \
    --build-arg=GOFLAGS \
    -f Dockerfile \
    -t l1fetech/adam -t l1fetech/adam:$VERSION \
    .
