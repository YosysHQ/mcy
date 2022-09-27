.. _testsetup:

Writing a test script
=====================

For each testbench in your test suite, you need to write a test script that MCY can call to run that testbench on a mutated design.
MCY will create a temporary directory, place a file named ``input.txt`` with a numbered list of mutations in it, run your test script, and expect to find the return status of the testbench for each mutation in a correspondingly numbered list in a file named ``output.txt``. (By default, the list will only contain a single entry, as this is the most straightforward to use. The parameter ``maxbatchsize`` can be set in the ``[test]`` section of ``config.mcy`` to increase the number of mutations included.)

.. A common optimization is that if your testbench is compiled, you can save compilation time by compiling multiple mutations into a single unit under test, but it requires modifying the testbench so that the mutation can be selected at execution time by passing an argument.

The test script will usually consist of three steps: exporting the mutated source, running the testbench, and reporting the result.

Exporting the Mutated Source
----------------------------

The first step is to obtain the modified source that includes the mutation(s) listed in ``input.txt``. The script ``create_mutated.sh`` makes this painless as long as your testbench can accept verilog or rtlil sources for the mutated module. When executing the test, MCY sets the variable ``$SCRIPTS`` to the path of the directory where you can find this script.

If you want to substitute a mutated verilog module with identical interface to the original, simply call it with no arguments:

.. code-block:: text

	bash $SCRIPTS/create_mutated.sh

This will result in a file named ``mutated.v`` containing a mutated module with the same name as the original module being created in the temporary directory where the test is executed (``task/<uuid>``).

If you want to use the ``maxbatchsize`` parameter to test multiple mutations in a single call of the test script, call the script with ``-c``:

.. code-block:: text

	bash $SCRIPTS/create_mutated.sh -c

This will result in a file ``mutated.v`` with a module of the same name but with an extra input signal ``mutsel``, which you can use to select which mutation to enable.

For more details about mutation generation, see :ref:`mutate`.

Running the Testbench
---------------------

Substitute the mutated module for the original in your testbench sources (usually by replacing the source file in the sources list with ``mutated.v``). If you are testing a single mutation at a time and did not pass the ``-c`` argument to ``create_mutated.sh``, you can now prepare and run your testbench as usual. The mutated module seamlessly replaces the original.

If you did enable multiple mutations to be included in the module, modify your testbench to add the ``mutsel`` input to the mutated module. If your testbench is compiled, add a way to pass the value of ``mutsel`` to the test at execution, e.g. via command line argument. The first column in ``input.txt`` is the number that selects the corresponding mutation. Run the testbench for each value appearing in ``input.txt``.

For example, if using ``iverilog`` (from the ``picorv32_primes`` example):

.. code-block:: text

	iverilog -o sim ../../sim_simple.v mutated.v
	while read idx mut; do
		vvp -N sim +mut=${idx} > sim_${idx}.out
	done < input.txt


Reporting the Result
--------------------

The results of the testbench run should be written to ``output.txt``. Each line should be the number identifying the mutation followed by a status. The status can be an arbitrary string not containing whitespace; however, any value not listed with the ``expect`` keyword in the test section of ``config.mcy`` is considered an error and will cause the mcy run to be aborted. Commonly used return values are "PASS", "FAIL" and sometimes "TIMEOUT".

If only a single mutation is evaluated at a time, the associated number is always ``1``. In this case, simply write ``1`` followed by the outcome of the testbench to ``output.txt``:

.. code-block:: text

	if $test_ok; then
		echo "1 PASS" > output.txt
	else
		echo "1 FAIL" > output.txt
	fi

If more than one mutation are evaluated in a run, each result should be on its own line, preceded by the corresponding value of ``mutsel``. The order does not matter.

For the previous example with ``iverilog``:

.. code-block:: text

	while read idx mut; do
		this_md5sum=$(md5sum sim_${idx}.out | awk '{ print $1; }')
		if [ $good_md5sum = $this_md5sum ]; then
			echo "$idx PASS" >> output.txt
		else
			echo "$idx FAIL" >> output.txt
		fi
	done < input.txt
