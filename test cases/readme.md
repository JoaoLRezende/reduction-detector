[Psyche-C][1] can be used to include here code fragments that reference external variables.

[1]: <http://cuda.dcc.ufmg.br/psyche-c/>

Note that we have a file named "compile_commands.json". That is an empty compilation database. We don't need anything there. All our test C source files are self-contained (don't include any non-standard header files) and do not require any specific compilation flags or macro definitions.
