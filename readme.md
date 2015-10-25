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
                "from": "/",
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

The optional match object specifies a simple string
replace on the value of the text, NOT a regex replace.

An example usage of psg would be:
```bash
# set the PS1 based on whether X is running
PROMPT_COMMAND=precmd
precmd() {
  if [ "$DISPLAY" ]; then
      LOC="${PWD/$HOME/\~}"
      GIT=$(/home/ted/.config/psg/git.sh)
      export PS1="$(cat ~/.config/psg/psg.json | psg2 user=$USER path=$LOC git=$GIT)"
  else
    export PS1='${debian_chroot:+($debian_chroot)}\[\033[01;32m\]\u@\h\[\033[00m\]:\[\033[01;34m\]\w\[\033[00m\]\$ '
  fi
}
```

```json
[
    {
        "name": "jobs",
        "forground": 1,
        "background": 2
    },
    {
        "name": "user",
        "forground": 253,
        "background": 243
    },
    {
        "name": "path",
        "forground": 253,
        "background": 238,
        "options" : {
            "match": {
                "from" : "/",
                "to" : "  "
            }
        }
    },
    {
        "name": "git",
        "forground": 52,
        "background": 6
    }
]
```
