package llm

import (
	"embed"
)

//go:embed moses.cpp/ggml-metal.metal moses.cpp/build/darwin/x86_64/*/lib/*.dylib*
var libEmbed embed.FS
