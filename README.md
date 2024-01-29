<div align="center">
  <img alt="adam" height="200px" src="https://lab.l1fe.tech/adam/assets/3325447/0d0b44e2-8f4a-4e99-9b52-a5c1c741c8f7">
</div>

# ADAM

Get up and running with ADAM LLMs locally.

### macOS

[Download](https://imadam.ai/download/adam-darwin.zip)

### Windows

Coming soon! For now, you can install ADAM on Windows via WSL2.

### Linux & WSL2

```
curl https://imadam.ai/install.sh | sh
```

[Manual install instructions](https://lab.l1fe.tech/adam/blob/main/docs/linux.md)

### Docker

The official [ADAM Docker image](https://hub.docker.com/r/l1fetech/adam) `l1fetech/adam` is available on Docker Hub.

### Libraries

- [adam-python](https://lab.l1fe.tech/adam/adam-python)
- [adam-js](https://lab.l1fe.tech/adam/adam-js)

## Quickstart

To run and chat with [Moses](https://imadam.ai/models/moses2):

```
adam run moses2
```

## Model library

ADAM supports a list of open-source models available on [imadam.ai/models](https://imadam.ai/models 'adam model library')

Here are some example open-source models that can be downloaded:

| Model              | Parameters | Size  | Download                       |
| ------------------ | ---------- | ----- | ------------------------------ |
| MOSES              | 7B         | 3.8GB | `adam run moses2`               |
| NEVIS              | 7B         | 4.1GB | `adam run nevis`               |


> Note: You should have at least 8 GB of RAM available to run the 7B models, 16 GB to run the 13B models, and 32 GB to run the 33B models.

## Customize a model

### Import from GGUF

ADAM supports importing GGUF models in the Modelfile:

1. Create a file named `Modelfile`, with a `FROM` instruction with the local filepath to the model you want to import.

   ```
   FROM ./vicuna-33b.Q4_0.gguf
   ```

2. Create the model in ADAM

   ```
   adam create example -f Modelfile
   ```

3. Run the model

   ```
   adam run example
   ```

### Import from PyTorch or Safetensors

See the [guide](docs/import.md) on importing models for more information.

### Customize a prompt

Models from the ADAM library can be customized with a prompt. For example, to customize the `moses2` model:

```
adam pull moses2
```

Create a `Modelfile`:

```
FROM moses2

# set the temperature to 1 [higher is more creative, lower is more coherent]
PARAMETER temperature 1

# set the system message
SYSTEM """
You are Donald Trump and you know everything about American history.
"""
```

Next, create and run the model:

```
adam create trump -f ./Modelfile
adam run trump
>>> hi
It's your favorite President. Me! Donald J. Trump. I know everything about American history - ask me something.
```

For more examples, see the [examples](examples) directory. For more information on working with a Modelfile, see the [Modelfile](docs/modelfile.md) documentation.

## CLI Reference

### Create a model

`adam create` is used to create a model from a Modelfile.

```
adam create mymodel -f ./Modelfile
```

### Pull a model

```
adam pull moses2
```

> This command can also be used to update a local model. Only the diff will be pulled.

### Remove a model

```
adam rm moses2
```

### Copy a model

```
adam cp moses2 mosesv2
```

### Multiline input

For multiline input, you can wrap text with `"""`:

```
>>> """Hello,
... world!
... """
I'm a basic program that prints the famous "Hello, world!" message to the console.
```

### Multimodal models

```
>>> What's in this image? /Users/adam/Desktop/smile.png
The image features a yellow smiley face, which is likely the central focus of the picture.
```

### Pass in prompt as arguments

```
$ adam run moses2 "Summarize this file: $(cat README.md)"
 ADAM is a lightweight, extensible framework for building and running language models on the local machine. It provides a simple API for creating, running, and managing models, as well as a library of pre-built models that can be easily used in a variety of applications.
```

### List models on your computer

```
adam list
```

### Start ADAM

`adam serve` is used when you want to start the `adam` engine without running the desktop application.

## Building

Install `cmake` and `go`:

```
brew install cmake go
```

Then generate dependencies:
```
go generate ./...
```
Then build the binary:
```
go build .
```

More detailed instructions can be found in the [developer guide](https://lab.l1fe.tech/adam/adam/blob/main/docs/development.md)


### Running local builds
Next, start the server:

```
./adam serve
```

Finally, in a separate shell, run a model:

```
./adam run moses2
```

## REST API

ADAM has a REST API for running and managing models.

### Generate a response

```
curl http://localhost:11434/api/generate -d '{
  "model": "moses2",
  "prompt":"Tell me the meaning of Exodus"
}'
```

### Chat with a model

```
curl http://localhost:11434/api/chat -d '{
  "model": "moses2",
  "messages": [
    { "role": "user", "content": "Tell me the meaning of Exodus" }
  ]
}'
```

See the [API documentation](./docs/api.md) for all endpoints.
