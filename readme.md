#PSG (PS1 Generator) -- a better powerline for bash

PSG provides a means of providing structured colors to
bash prompts so that properly blended powerlines work.

PSG reads a configuration from stdin in the form of a
json strucure like the following:
```json
[
    {
        "name" : "<a name in quotes>",
        "forground": "<256 color terminal color>",
        "background": "<256 color terminal color>",
        "default": "<any quoted text>", //optional
        "options" : { //optional
            "match" : {
                from: "/",
                "to": "\\"
            }
        }
    }
    // repeat many of these
]
```

psg then reads input values from the command line in the
form NAME=VALUE, where NAME is one of the names given in
the json block on stdin, and value is what will actually
be printed in that location. An example invocation would
be:
```sh
cat ~/.psg.json | psg user=$USER path=${PWD/$HOME/\~}
```

which would display (on a test machine)
ted  ~ 
though it will be colored based on the config from stdin


