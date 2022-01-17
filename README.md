# Mutation Cover with Yosys

`mcy` is a new tool to help digital designers and project managers understand and improve testbench coverage.

*If you have a testbench, and it fails, you know you have a problem. But if it passes, you know nothing if you don’t know what your testbench is actually testing for.*

![MCY Overview Diagram](https://github.com/YosysHQ/mcy/raw/master/docs/images/mcy.png)

Given a self checking testbench, `mcy` generates 1000s of mutations by modifying individual signals in a post synthesis netlist. These mutations are then filtered using Formal Verification techniques, keeping only those that can cause an important change in the design’s output.

All mutated designs are run against the testbench to check that the testbench will detect and fail for a relevant mutation. The testbench can then be improved to get 100% complete coverage.

`mcy gui` can be used to explore the results and find where coverage needs improvement.

![MCY GUI Screenshot](https://github.com/YosysHQ/mcy/raw/master/docs/images/mcy-gui.png)

`mcy dash` provides at a glance overview as the tool runs. It is browser-based to easily take advantage of cloud compute.

![MCY Dash Screenshot](https://github.com/YosysHQ/mcy/raw/master/docs/images/mcy-dash.png)

**For more information please contact matt@yosyshq.com**

## Getting Started
### Installing Tabby CAD Suite or OSS CAD Suite

`mcy` is part of the [Tabby CAD Suite](https://www.yosyshq.com/tabby-cad-datasheet) and the [OSS CAD Suite](https://github.com/YosysHQ/oss-cad-suite-build)! The easiest way to use `mcy` is to install the binary software suite, which contains all required dependencies.

* [Contact YosysHQ](https://www.yosyshq.com/contact) for a [Tabby CAD Suite](https://www.yosyshq.com/tabby-cad-datasheet) Evaluation License and download link
* OR go to https://github.com/YosysHQ/oss-cad-suite-build/releases to download the free OSS CAD Suite
* Follow the [Install Instructions on GitHub](https://github.com/YosysHQ/oss-cad-suite-build#installation)

Make sure to get a Tabby CAD Suite Evaluation License if you need features such as industry-grade SystemVerilog and VHDL parsers!

For more information about the difference between Tabby CAD Suite and the OSS CAD Suite, please visit https://www.yosyshq.com/tabby-cad-datasheet.

### Installing from Source

Install [SymbiYosys](http://symbiyosys.readthedocs.io/) and its dependencies first.

Install `mcy`:

```
sudo make install
```

Run an example project (may run for a while):

```
cd examples/bitcnt
mcy init
mcy run -j8
```

The examples additionally require Icarus Verilog (https://github.com/steveicarus/iverilog) to run the testbench under test.

## Tutorial

Create a new directory with a `config.mcy` file. For example:

```
[options]
size 100

[script]
read_verilog verilog/picorv32.v
prep -top picorv32

[files]
picorv32.v

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

[report]
if tags("!NOC"):
    print("Coverage: %.2f%%" % (100.0*tags("COVERED")/tags("!NOC")))

[test sim_simple]
maxbatchsize 10
expect PASS FAIL
run bash $PRJDIR/scripts/sim_simple.sh

[test eq_bmc]
maxbatchsize 10
expect PASS FAIL
run bash $PRJDIR/scripts/eq_bmc.sh

[test eq_sim3]
maxbatchsize 10
expect TIMEOUT FAIL
run bash $PRJDIR/scripts/eq_sim3.sh
```

Create the referenced files and run `mcy init`.

This will create a `database/` directory, run the Yosys script and create
`database/design.il`, create `size` mutations and store them in `database/db.sqlite3`.

Running `mcy reset` re-runs the `[logic]` section for all mutations, imports
additional mutations if the value of the `size` option has been increased,
resets the "running" status of all tests, removes the `tasks/` directory, and
prints some statistics.

Running `mcy status` prints the statistics from `mcy reset` but will
not make any changes to the database.

Running `mcy list` will list all mutations and their current tags.

Running `mcy list --details` will print more details for each mutation.

Running `mcy run -j8` will run the mutation cover analysis, with a maximum
of 8 concurently running jobs.

Running `mcy task [-v] [-k] <test> <id_or_tag>..` will (re-)run the specified
test with the given mutations. This is mostly used during development of test
scripts. `-v` displays the command and its output on the console, and with `-k`
the task directory `tasks/<task_id>` will not be removed when the task
completes.

Running `mcy source <filename>` prints the original HDL source, annotated with
coverage information. (This is hardcoded to use the tags `COVERED` and
`UNCOVERED`.) This is using the copy of the specified source file that was
placed in the database on `mcy init`. (This requires a `[files]` corresponding
entry in the config file.) Use `mcy source <attr_filename> <fs_filename>`
to use a source file from the file system instead.

Running `mcy dash` launches a web-based dashboard that can be used to launch
jobs and monitor progress.

Running `mcy gui` launches a GUI application that can be used to investigate
the database.

Finally, `mcy purge` removes the `database/` and `tasks/` directories.

### Logic

The `[logic]` section contains python code that defines how `mcy` should
run the individual tests. The following special functions are available:

`tag(tagname)`: Mark the current mutation with the specified tag

`result(test_with_args)`: Run the specified test and return the
result, or use a cached result if the test had been run previously.

`rng(N)`: Return a deterministic pseudo-random integer in the range 0..N-1.

### Report

The `[report]` section contains python code for printing a report based
on the data collect via the tagging mechanism described above.

`tags()` returns the total number of mutations that have any tag assigned
to them.

`tags("FOO")` returns the number of mutations that have the tag `FOO` assigned.

`tags("!FOO")` returns the number of mutations that have at least one tag
assigned, but not the `FOO` tag.

### Tests

Each test has its own configuration file section, `[test testname]`.

If `maxbatchsize` is set to a value > 1 then `mcy` will automatically create
batches of up to the specified number of mutations, and then pass them all
to a single invocation of the test. (The default `maxbatchsize` value is `1`.)

Running the test means running the command specified with `run`, with any
additional arguments to the test appended to the command.

The command is run in the `tasks/<taskid>` subdirectory, with the environment
variable `TASK` set to the task id, `TASKDIR` set to the directory, and `PRJDIR`
set to the project directory.

The environment variable `KEEPDIR` will be set to `1` if the task is run with
`mcy task -k`, and `0` otherwise. This can be used for example to only create
VCD traces if the task directory is kept.

The command is then given an input in `input.txt`, formatted as following:

```
1 <mutation_1>
2 <mutation_2>
3 <mutation_3>
...
```

And it will procude an output in `output.txt`, formatted as following:

```
1 <result_1>
2 <result_2>
3 <result_3>
...
```

The numbers are not mutation IDs but a task-local numbering, starting fresh
at 1 for each task. The order of the output lines does not matter.

The environment variable `MUTATIONS` contains the list of mutation IDs for the
task. This is mostly useful for debug purposes. Ideally a test should not
need to know the mutation IDs of the mutations it tests.

Standard output of the command will be redirected to `logfile.txt`, unless
the command is run from `mcy task -v`. Standard error always stays connected
to the console.

A test with `expect` setting will cause a runtime error if the test returns
anything else than one of the expected values.
