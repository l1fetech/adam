#!/bin/sh

set -eu

export VERSION=${VERSION:-$(git describe --tags --first-parent --abbrev=7 --long --dirty --always | sed -e "s/^v//g")}
export GOFLAGS="'-ldflags=-w -s \"-X=github.com/l1fetech/adam/version.Version=$VERSION\" \"-X=github.com/l1fetech/adam/server.mode=release\"'"

docker build \
    --load \
    --platform=linux/arm64,linux/amd64 \
    --build-arg=VERSION \
    --build-arg=GOFLAGS \
    -f Dockerfile \
    -t l1fetech/adam:$VERSION \
    .

docker build \
    --load \
    --platform=linux/amd64 \
    --build-arg=VERSION \
    --build-arg=GOFLAGS \
    --target runtime-rocm \
    -f Dockerfile \
    -t l1fetech/adam:$VERSION-rocm \
    .
