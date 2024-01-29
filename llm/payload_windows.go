package llm

import (
	"embed"
)

//go:embed moses.cpp/build/windows/*/*/lib/*.dll*
var libEmbed embed.FS
