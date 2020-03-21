
Tutorial
--------

This guide will explain how to set up a project from scratch using the bit-counter project in ``examples/bitcnt`` as an example. A shorter overview of the project is also available in `this youtube video`_.

_this youtube video: https://youtu.be/NKzqRum1ksg

You should start out with the following files:

``bitcnt_tb.v``: a self-checking testbench (or a set of testbenches) for which you wish to measure coverage. This is not restricted to HDL testbenches but can be any kind of test that can be launched without manual intervention and can return a PASS/FAIL result.

``bitcnt.v``: this represents the synthesizable design (or portion of the design) that is going to be mutated by ``mcy``. This design can comprise multiple modules/files. If your design is large, or requires many steps of formal verification, you should split your design into multiple parts to be mutated separately. The "mutation top level module" does not have to be the same as the module under test in the testbench, it can be any module in its submodule hierarchy.

After completing this tutorial, your files should match the contents of the ``examples/bitcnt`` directory.

Configuration file
~~~~~~~~~~~~~~~~~~
Create a new directory for your project, and inside it create a new file called ``config.mcy``. This is the main MCY configuration file. There are several required sections in a MCY config file which will be progressively filled in in the next steps. Insert the first five headers for these sections now:

.. code-block:: text

	[options]

	[script]

	[files]

	[logic]

	[report]

The first setting can already be added now, under the ``[options]`` section:

.. code-block:: text

	[options]
	size 10

This is the number of mutations that ``mcy`` will generate. To begin with, this very low is chosen number during setup of the project so that the test runs are quick. Once everything is running correctly, we will increase the size of the mutation set.

Mutable netlist generation
~~~~~~~~~~~~~~~~~~~~~~~~~~
Yosys introduces mutations in a (coarse-grain) synthesized netlist. In a first step, we need to tell Yosys how to build this netlist.

In the ``[script]`` section, fill in the steps to read your design in Yosys:

.. code-block:: text

	[script]
	read_verilog bitcnt.v
	prep -top bitcnt

This should be a series of ``read`` commands, one per file. If your design is large, include only the subset of files necessary to build the module that should be mutated. When all sources are read in, the line ``prep -top bitcnt`` runs coarse grain synthesis on the module to be mutated designated by ``-top``.

You can test your script either in interactive mode in Yosys or by saving it in a file ``script.ys`` (without the ``[script]`` header) and running ``yosys script.ys``.

In the ``[files]`` section, list the paths to the files required by the script:

.. code-block:: text

	[files]
	bitcnt.v


Script to run the testbench on the mutated module
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Create a file named ``test_sim.sh``. This will run the existing testbench on the mutated
design. The beginning of the file will be boilerplate that writes out the design with the
selected mutation to verilog:

.. code-block:: text

	#!/bin/bash

	exec 2>&1
	set -ex

	{
		echo "read_ilang ../../database/design.il"
		while read -r idx mut; do
			echo "mutate ${mut#* }"
		done < input.txt
		echo "write_verilog -attr2comment mutated.v"
	} > mutate.ys

	yosys -ql mutate.log mutate.ys


When mcy runs, the tests will be executed in a temporary directory ``tests/<uuid>``, so
the paths should be relative to this location.

Next, insert whatever code will run your testbench as usual, but replacing the original
source for the module to be mutated (``bitcount.v``) with the mutated source
(``mutated.v``). The bitcnt testbench uses ``iverilog``:

.. code-block:: text

	iverilog -o sim ../../bitcnt_tb.v mutated.v
	vvp -n sim > sim.out


Finally, write the status returned by the testbench to the file ``output.txt``:

.. code-block:: text

	if grep PASS sim.out && ! grep ERROR sim.out; then
		echo "1 PASS" > output.txt
	elif ! grep PASS sim.out && grep ERROR sim.out; then
		echo "1 FAIL" > output.txt
	else
		echo "1 ERROR" > output.txt
	fi

	exit 0

The ``1`` before the status is the test index. For tests with significant setup costs, it
is possible to test multiple mutations in a single execution, in which case this number
identifies the test run. Here we run each test individually so the index is always 1.

You can test that this portion works correctly as follows:

- create the directories ``database`` and ``tasks/test`` inside the project directory

  Note: these directories will get deleted when you run ``mcy`` so do not save any important files in them.

- add ``write_ilang database/design.il`` to the end of the ``script.ys`` file created earlier

- run the following commands:

.. code-block:: text

	yosys script.ys
	cd tasks/test
	echo "1 mutate -mode none" > input.txt
	bash ../../test_sim.sh

- verify that the file ``output.txt`` was created and contains ``1 PASS``.

If everything is working, add the following section to the bottom of ``config.mcy``:

.. code-block:: text

	[test test_sim]
	expect PASS FAIL
	run bash $PRJDIR/test_sim.sh

This tells ``mcy`` that the test ``test_sim`` exists and how to run it. If ``output.txt``
ever contains a value not listed under ``expect`` when this test is run, the entire
``mcy`` process will be aborted.

Setting up the formal equivalence test
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
This is the most work-intensive part of an ``mcy`` project, but also what makes ``mcy`` special. To know whether the testbench under test *should* return PASS or FAIL, we will set up a formal property check that can conclusively determine whether a mutation can affect the output of the module in a relevant way.

The advantage of using formal methods is that they will exhaustively explore all possible input combinations, which is prohibitive for a simulation testbench for most non-trivial designs due to combinatorial explosion. But the ``mcy`` approach is also less difficult than outright formally verifying the design, as it is generally easier to describe whether a change to the output is "important" than to describe the correct behaviour directly.

Unlike in the previous test where we asked Yosys to export the mutated module with the same interface as the original module so we could seamlessly replace it in the testbench, here we will use a different command to get a module where we can enable or disable the mutation at will based on an input signal ``mutsel``.

Create a file ``test_eq.sh`` and add the following script:

.. code-block:: text

	#!/bin/bash

	exec 2>&1
	set -ex

	{
		echo "read_ilang ../../database/design.il"
		while read -r idx mut; do
			echo "mutate -ctrl mutsel 8 ${idx} ${mut#* }"
		done < input.txt
		echo "write_ilang mutated.il"
	} > mutate.ys

	yosys -ql mutate.log mutate.ys

Next, we will create a miter circuit that instatiates both the original and the mutated module. Create a file named ``test_eq.sv`` and enter the following code:

.. code-block:: text

	module miter (
		input [63:0] ref_din_data,
		input [63:0] uut_din_data,
		input [ 2:0] din_func
	);
		wire [63:0] ref_dout_data;
		wire [63:0] uut_dout_data;

		bitcnt ref (
			.mutsel    (1'b 0),
			.din_data  (ref_din_data),
			.din_func  (din_func),
			.dout_data (ref_dout_data)
		);

		bitcnt uut (
			.mutsel    (1'b 1),
			.din_data  (uut_din_data),
			.din_func  (din_func),
			.dout_data (uut_dout_data)
		);

	endmodule

This instantiates the ``bitcnt`` module twice, once with the mutation disabled (``ref``) and once with the mutation enabled (``uut``). Next, we will add ``assert`` and ``assume`` statements that express under which conditions we expect which outputs to be unmodified.

The ``bitcnt`` module has multiple modes of operation selected by the input ``din_func``. The LSB ``din_func[0]`` selects between 32-bit and 64-bit operand mode, and the MSBs ``din_func[2:1]`` choose between three counting modes, count leading zeros (CLZ), count trailing zeros (CTZ), or popcount (CNT). The fourth option, ``din_func[2:1]==2'b11`` is not a valid operation.

The goal is to be as precise as possible about the conditions under which we expect the same output. Therefore we will never check anything in the case of the unused opcode ``din_func[2:1] == 2'b11``. We will also disambiguate between the 32 and 64-bit modes and allow the upper input and output bits of ``uut`` and ``ref`` to not be identical in 32-bit mode.

At the end of the miter module (before ``endmodule``), insert the following code:

.. code-block:: text

	always @* begin
		casez (din_func)
			3'b11z: begin
				// unused opcode: don't check anything
			end
			3'bzz1: begin
				// 32-bit opcodes, only constrain lower 32 bits and only check lower 32 bits
				assume (ref_din_data[31:0] == uut_din_data[31:0]);
				assert (ref_dout_data[31:0] == uut_dout_data[31:0]);
			end
			3'bzz0: begin
				// 64-bit opcodes, constrain all 64 input bits and check all 64 output bits
				assume (ref_din_data == uut_din_data);
				assert (ref_dout_data == uut_dout_data);
			end
		endcase
	end

We will use SymbiYosys to check these formal properties. Create the file ``test_eq.sby`` and enter the following configuration:

.. code-block:: text

	[options]
	mode bmc
	depth 1
	expect pass,fail

	[engines]
	smtbmc yices

	[script]
	read_verilog -sv test_eq.sv
	read_ilang mutated.il
	prep -top miter
	fmcombine miter ref uut
	flatten
	opt -fast

	[files]
	test_eq.sv
	mutated.il

You can consult the `SymbiYosys documentation`_ for detailed information about how to set up an ``sby`` project. Points of note here are:

- The ``bitcnt`` module is combinatorial, so we can use a bounded model check with a single step.

- The additional steps ``fmcombine``, ``flatten`` and ``opt`` in the script section are not mandatory but increase the speed of the check.

- All files used are assumed to be present in the directory in which the test is run.

.. _SymbiYosys documentation: https://symbiyosys.readthedocs.io/en/latest/quickstart.html#first-step-a-simple-bmc-example

You can test your ``sby`` setup in the ``tasks/test`` directory with the already created ``input.txt`` as follows:

.. code-block:: text

	cd tasks/test
	ln -s ../../test_eq.sv ../../test_eq.sby .
	bash ../../test_eq.sh
	sby -f test_eq.sby

As we are once again testing the "do nothing" mutation, this should return ``PASS``. If it works correctly, we can complete the script for this test to run ``sby`` and extract the return value. Append the following to ``test_eq.sh``:

.. code-block:: text

	ln -fs ../../test_eq.sv ../../test_eq.sby .

	sby -f test_eq.sby
	gawk "{ print 1, \$1; }" test_eq/status >> output.txt

	exit 0

You can check once more that running ``bash ../../test_eq.sh`` inside ``tasks/test`` works correctly and writes ``1 PASS`` to ``output.txt``. Note that the script appends data to this file and an identical line might already exist from previous runs, so verify that a new line is added with the execution.

Finally, set up the configuration for this test at the end of ``config.mcy``:

.. code-block:: text

	[test test_eq]
	expect PASS FAIL
	run bash $PRJDIR/test_eq.sh

Tagging Logic
~~~~~~~~~~~~~

Now that we have set up the two tests, we need to tell ``mcy`` how we want to analyze the results. With two tests, there are only four possible outcomes, which we can each assign a tag:

- both tests fail: the testbench accurately detects the problem, i.e. the mutation is COVERED.

- the simulation testbench passes but the equivalence test fails: the testbench does not find the problem, i.e. the mutation is UNCOVERED.

- the simulation testbench passes and the equivalence test passes: the mutation does not introduce a relevant change to the functionality of the module (NOCHANGE).

- the simulation testbench fails but the equivalence test passes: the equivalence test must not have been set up correctly, and there is a gap between formal description and expected behaviour (FMGAP).

Declare these four tags in the ``[options]`` section:

.. code-block:: text

	[options]
	size 10
	tags COVERED UNCOVERED NOCHANGE FMGAP

Then, under the ``[logic]`` section, describe how to tag the tests:

.. code-block:: text

	sim_okay = result("test_sim") == "PASS"
	eq_okay = result("test_eq") == "PASS"

	if sim_okay and not eq_okay:
	    tag("UNCOVERED")
	elif not sim_okay and not eq_okay:
	    tag("COVERED")
	elif sim_okay and eq_okay:
	    tag("NOCHANGE")
	else:
	    tag("FMGAP")

This section essentially defines a python function, and can use the predefined functions ``result("<name>")`` (where ``<name>`` is a test defined in a ``[test <name>]`` section) and ``tag("<name>")`` (for any tag defined under ``tags`` in the ``[options]`` section). A single mutation can be tagged with multiple tags, or with no tags at all.

When you have multiple tests of differing length, you can use lazy evaluation to run tests conditionally. For a given mutation, a test is only executed when the ``[logic]`` section calls ``result()``. (To see how this feature is used see the other example project, ``picorv32_primes``.)

Finally, fill in the ``[report]`` section as follows:

.. code-block:: text

	[report]
	if tags("FMGAP"):
	    print("Found %d mutations exposing a formal gap!" % tags("FMGAP"))
	if tags("COVERED")+tags("UNCOVERED"):
	    print("Coverage: %.2f%%" % (100.0*tags("COVERED")/(tags("COVERED")+tags("UNCOVERED"))))

This is again a section that defines a python function. Here, the function ``tags("<name>")`` can be used to obtain the number of mutations tagged with a given tag.
If there is a formal gap, this is highly problematic so this will be reported first. Secondly, we print a coverage metric calculated as the percent of covered mutations out of all mutations that induce a relevant design change, i.e. both those tagged as covered and as uncovered.

Running mcy
~~~~~~~~~~~

Now the ``mcy`` project is fully set up. Delete the temporary folders ``database`` and ``tasks`` we created for testing by running:

.. code-block:: text

	mcy purge

Then, execute ``mcy``:

.. code-block:: text

	mcy init
	mcy run

As there are only a few tests requested initially, this should complete quickly. Running in sequential mode (without ``-j`` argument) makes it more obvious which test is the cause in case of error.

If this initial test run completes successfully and prints a coverage metric, you can increase the number of mutations at the beginning of ``config.mcy``:

.. code-block:: text

	[options]
	size 1000

This time, the tests will take longer to run, so enable parallel runs (replace ``$(nproc)`` with the number of cores to use):

.. code-block:: text

	mcy reset
	mcy run -j$(nproc)

``reset`` will keep the existing results for the previously tested mutations but add more mutations to reach the new requested size.

While the tests are being run, in a second terminal, you can run (in the base project directory where your ``config.mcy`` is located)

.. code-block:: text

	mcy dash

and open the provided address in your browser to follow progress in the dashboard. This can be especially of interest when running tests on a remote server.

Once the tests complete, you can use:

.. code-block:: text

	mcy gui

to explore visually the hotspots in your code where coverage gaps exist. This is currently hardcoded to use the tag names "COVERED" and "UNCOVERED".

A similar, command-line-only view is produced by:

.. code-block:: text

	mcy source bitcnt.v

Positive numbers in the left-hand column indicate mutations tagged as COVERED, negative numbers indicate UNCOVERED.

You can try to improve the testbench in ``bitcnt_tb.v`` to achieve better coverage. After modifying this file, don't forget to invalidate old results by running:

.. code-block:: text

	mcy purge

As mutations are generated randomly, the better your coverage, the larger the size required to find uncovered cases. If you reach 100%, try increasing the size further.
