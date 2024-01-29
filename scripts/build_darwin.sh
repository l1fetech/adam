#!/bin/sh

set -e

export VERSION=${VERSION:-$(git describe --tags --first-parent --abbrev=7 --long --dirty --always | sed -e "s/^v//g")}
export GOFLAGS="'-ldflags=-w -s \"-X=github.com/jmorganca/adam/version.Version=$VERSION\" \"-X=github.com/jmorganca/adam/server.mode=release\"'"

mkdir -p dist

for TARGETARCH in arm64 amd64; do
    rm -rf llm/moses.cpp/build
    GOOS=darwin GOARCH=$TARGETARCH go generate ./...
    CGO_ENABLED=1 GOOS=darwin GOARCH=$TARGETARCH go build -o dist/adam-darwin-$TARGETARCH
    CGO_ENABLED=1 GOOS=darwin GOARCH=$TARGETARCH go build -cover -o dist/adam-darwin-$TARGETARCH-cov
done

lipo -create -output dist/adam dist/adam-darwin-arm64 dist/adam-darwin-amd64
rm -f dist/adam-darwin-arm64 dist/adam-darwin-amd64
if [ -n "$APPLE_IDENTITY" ]; then
    codesign --deep --force --options=runtime --sign "$APPLE_IDENTITY" --timestamp dist/adam
else
    echo "Skipping code signing - set APPLE_IDENTITY"
fi
chmod +x dist/adam

# build and optionally sign the mac app
npm install --prefix app
if [ -n "$APPLE_IDENTITY" ]; then
    npm run --prefix app make:sign
else 
    npm run --prefix app make
fi
cp app/out/make/zip/darwin/universal/ADAM-darwin-universal-$VERSION.zip dist/ADAM-darwin.zip

# sign the binary and rename it
if [ -n "$APPLE_IDENTITY" ]; then
    codesign -f --timestamp -s "$APPLE_IDENTITY" --identifier ai.adam.adam --options=runtime dist/adam
else
    echo "WARNING: Skipping code signing - set APPLE_IDENTITY"
fi
ditto -c -k --keepParent dist/adam dist/temp.zip
if [ -n "$APPLE_IDENTITY" ]; then
    xcrun notarytool submit dist/temp.zip --wait --timeout 10m --apple-id $APPLE_ID --password $APPLE_PASSWORD --team-id $APPLE_TEAM_ID
fi
mv dist/adam dist/adam-darwin
rm -f dist/temp.zip
