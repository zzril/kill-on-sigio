kill-on-sigio
=============

Set up for a process to be killed by writing to a specific file.

Useful e. g. for implementing alarm clocks via `cron`, so you have a reliable way of stopping the alarm without having to resort to some fancy `pgrep`.

Example
-------

```sh
mkfifo stopfile
make
./bin/kill-on-sigio stopfile ffplay -nodisp -autoexit ~/Music/YouTube/dQw4w9WgXcQ.m4a 2> /dev/null &
printf '' > stopfile
```

