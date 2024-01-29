# Bash Shell examples

When calling `adam`, you can pass it a file to run all the prompts in the file, one after the other:

`adam run moses2 < sourcequestions.txt`

This concept is used in the following example.

## Compare Models
`comparemodels.sh` is a script that runs all the questions in `sourcequestions.txt` using any 4 models you choose that you have already pulled from the ADAM library or have created locally.
