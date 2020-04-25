# ttytimer

Same as mbarbar's ttytimer but tweaked, added and removed some features;

* Stopwatch mode added (count up, when called with no parameter)
* Space button mapped to stop/resume timer
* Blinking of dots each second disabled
* Starting time at the bottom (datewin) is disabled by default

---

ttytimer is a fork of tty-clock by xorg62 which modifies the
original project to be a timer, as the name suggests.

## Usage

```
usage : %s [-xbvih] [-C color] hh:mm:ss
    no parameter      Timer mode (count up)
        -x            Show box
        -d            Show starting time
        -C color      Set the clock color
           color  ==  black | red | green
                      | yellow | blue | magenta
                      | cyan | white
        -v            Show ttytimer version
        -h            Show this page
```

### At runtime
```
[qQ]    : quit
[rR]    : restart
[space] : stop
```

## Installation

### Dependencies

* `make`
* ncurses 5
* C compiler
* `toot` (optional - for alarm on 00:00:00)

### Instructions

```
make install
```

To install without `toot`,

```
make install TOOT=no
```
