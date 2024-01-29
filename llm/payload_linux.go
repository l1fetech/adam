package llm

import (
	"embed"
)

//go:embed moses.cpp/build/linux/*/*/lib/*.so*
var libEmbed embed.FS
