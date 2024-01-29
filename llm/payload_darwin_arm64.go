package llm

import (
	"embed"
)

//go:embed moses.cpp/ggml-metal.metal moses.cpp/build/darwin/arm64/*/lib/*.dylib*
var libEmbed embed.FS
