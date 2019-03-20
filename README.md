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
if result("sim_simple", ("PASS", "FAIL")) != "PASS":
    tag("COVERED")
    return

if result("eq_sim3 10", ("PASS", "TIMEOUT")) != "PASS":
    tag("UNCOVERED")
    return

if result("eq_bmc", ("PASS", "FAIL")) != "PASS":
    tag("UNCOVERED")
    return

if result("eq_sim3 500", ("PASS", "TIMEOUT")) != "PASS":
    tag("UNCOVERED")
    return

tag("NOC")

[test sim_simple]
maxbatchsize 1
run bash scripts/sim_simple.sh

[test eq_bmc]
maxbatchsize 1
run bash scripts/eq_bmc.sh

[test eq_sim3]
maxbatchsize 1
run bash scripts/eq_sim3.sh
```

Create the referenced files and run `mcy init`.

This will create a `database/` directory, run the Yosys script and create
`database/design.il`, create 100 mutations and store them in `database/db.sqlite3`.


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

`result(test_with_args, [expected_results])`: Run the specified test and return the
result (or use a cached result if the test had been run previously). Also, trigger a
runtime error if the result returned by the test is not in `expected_results`.

`rng(N)`: Return a deterministic pseudo-random integer in the range 0..N-1.

## Tests

Each test has its own configuration file section, `[test testname]`.

If `maxbatchsize` is set to a value > 1 then `mcy` will automatically create
batches of up to the specified number of mutations, and then pass them all
to a single invocation of the test.

Running the test means running the command specified with `run`, with any
additional arguments to the test appended to the command.

This command is then given an input, formatted as following:

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

## Database Schema

```
CREATE TABLE mutations (
  id INTEGER PRIMARY KEY,
  mutation STRING.
);

CREATE TABLE results (
  mutation_id INTEGER,
  test STRING,
  result STRING
);

CREATE TABLE tags (
  mutation_id INTEGER,
  tag STRING
);

CREATE TABLE queue (
  mutation_id INTEGER,
  test STRING
);
```