# Mutation Cover with Yosys

**Under Construction**

## Quickstart

Create a new directory with a `config.mcy` file. For example:

```
[options]
size 100

[script]
read_verilog verilog/picorv32.v
prep -top picorv32

[logic]
if result("sim_simple") == "FAIL":
    tag("COVERED")
    return

if result("eq_sim3 10") == "FAIL":
    tag("UNCOVERED")
    return

if result("eq_bmc") == "FAIL":
    tag("UNCOVERED")
    return

if result("eq_sim3 500") == "FAIL":
    tag("UNCOVERED")
    return

tag("NOC")

[test sim_simple]
maxbatchsize 10
expect PASS FAIL
run bash scripts/sim_simple.sh

[test eq_bmc]
maxbatchsize 10
expect PASS FAIL
run bash scripts/eq_bmc.sh

[test eq_sim3]
maxbatchsize 10
expect TIMEOUT FAIL
run bash scripts/eq_sim3.sh
```

Create the referenced files and run `mcy init`.

This will create a `database/` directory, run the Yosys script and create
`database/design.il`, create `size` mutations and store them in `database/db.sqlite3`.

Running `mcy update` will re-run the `[logic]` section for all mutations and output
some statistics.

Running `mcy status` will only output the statistics from `mcy update` but will
not make any changes to the database.

Running `mcy list` will list all mutations and their current tags.

Running `mcy list --details` will print more details for each mutation.

Running `mcy run -j8` will run the mutation cover analysis, with a maximum
of 8 concurently running jobs.

Running `mcy dash` launches a web-based dashboard that can be used to launch
jobs and monitor progress.

Finally, `mcy gui` launches a GUI application that can be used to investigate
the database.

## Logic

The `[logic]` section contains a python function that defines how `mcy` should
run the individual tests. The following special functions are available:

`tag(tagname)`: Mark the current mutation with the specified tag

`result(test_with_args)`: Run the specified test and return the
result, or use a cached result if the test had been run previously.

`rng(N)`: Return a deterministic pseudo-random integer in the range 0..N-1.

## Tests

Each test has its own configuration file section, `[test testname]`.

If `maxbatchsize` is set to a value > 1 then `mcy` will automatically create
batches of up to the specified number of mutations, and then pass them all
to a single invocation of the test. (The default `maxbatchsize` value is `1`.)

Running the test means running the command specified with `run`, with any
additional arguments to the test appended to the command.

The command is then given an input, formatted as following:

```
<id_1>: <mutation_1>
<id_2>: <mutation_2>
<id_3>: <mutation_3>
...
```

And it will procude an output, formatted as following:

```
<id_1>: <result_1>
<id_2>: <result_2>
<id_3>: <result_3>
...
```

(The order of the lines does not matter.)

A test with `expect` setting will cause a runtime error if the test returns
anything else than one of the expected values.
