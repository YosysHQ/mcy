Writing the equivalence check
=============================

For the equivalence check, not only do you need to write a test script similar to those for your test suite, but you will also need to write the miter circuit for the equivalence check itself.

Creating the miter circuit
--------------------------

The equivalence check is done with a miter circuit, which instantiates the same module (exported with the additional control input) twice, with mutsel values ``0`` (original) and ``1`` (mutated).

The contents of the equivalence check are specific to your design. As an example, let us consider a synchronous, resettable module named ``example`` with two inputs ``in1`` and ``in2``, and an associated flow control signal ``in_valid``, as well as an output ``out`` that has associated handshake control signals ``out_valid`` and ``out_ready`` (the latter being an input signal). The specification contains the usual stipulations that the values of ``in1`` and ``in2`` are ignored on cycles where ``in_valid`` is low, and the value of ``out`` is correct whenever ``out_valid`` is high and remains stable as long as ``out_ready`` is low.

First, instantiate the module twice, with ``mutsel`` bound to ``0`` for the reference module, and ``mutsel`` bound to ``1`` for the mutated module.

For any input of the modules, create two sets of inputs to the miter module. These inputs will be constrained using assumptions whenever necessary. For signals where the values can never diverge without affecting the behavior of the modules, such as e.g. the clock and reset signals, a single input connected to both modules is sufficient.

For outputs, create two sets wires to connect them. These wires will serve to compare the outputs to check if they are equivalent.

.. code-block:: text

	module miter (
		input clk,
		input rst,
		input ref_in1,
		input ref_in2,
		input ref_in_valid,
		input ref_out_ready,
		input uut_in1,
		input uut_in2,
		input uut_in_valid,
		input uut_out_ready
	);

		wire ref_out;
		wire ref_out_valid;

		wire uut_out;
		wire uut_out_valid;

		example ref (
			.clk(clk),
			.rst(rst),
			.in1(ref_in1),
			.in2(ref_in2),
			.in_valid(ref_in_valid),
			.out(ref_out),
			.out_valid(ref_out_valid),
			.out_ready(ref_out_ready),
			.mutsel(1'b0)
		);

		example uut (
			.clk(clk),
			.rst(rst),
			.in1(uut_in1),
			.in2(uut_in2),
			.in_valid(uut_in_valid),
			.out(uut_out),
			.out_valid(uut_out_valid),
			.out_ready(uut_out_ready),
			.mutsel(1'b1)
		);

	endmodule

Next, add assumptions that inputs are identical whenever the module behavior should be influenced by the input value. For the flow control inputs ``in_valid`` and ``out_ready``, this is at any time. For ``in1`` and ``in2``, only assume identical inputs when ``in_valid`` is high - the specification says that the value should not matter otherwise.

Add assertions that the outputs are equivalent whenever the specification indicates that a value should be valid. For the flow control output ``out_valid`` this is at any time, for the output ``out`` only when ``out_valid`` is high.

If your module contains internal state that can affect results, you may need to also constrain the initial state, e.g. by assuming that the reset is enabled in the first cycle.

.. code-block:: text

	module miter (
		...
	);

		...

		example ref (
			...
		);

		example uut (
			...
		);

		initial assume (rst);

		always @(posedge clk) begin
			if (!rst) begin
				assume (ref_in_valid == uut_in_valid);
				assume (ref_out_ready == uut_out_ready);
				if (ref_in_valid) begin
					assume (ref_in1 == uut_in1);
					assume (ref_in2 == uut_in2);
				end

				assert (ref_out_valid == uut_out_valid);
				if (ref_out_valid) begin
					assert (ref_out == uut_out);
				end
			end
		end

	endmodule

Be as precise as possible in your assume and assert statements! If you overconstrain, you are likely to declare that some mutations do not affect the behavior of the module when there are valid circumstances in which they would change the behavior, but you have excluded them from consideration. If you underconstrain your design, you will probably find that the equivalence check fails to find the original module equivalent to itself, but this is much easier to detect: Simply run the equivalence check on the no-change mutation, which will always be included in the mutation database with ID 1.


Setting up the test script for the equivalence check
----------------------------------------------------

Mutation export
~~~~~~~~~~~~~~~
For the equivalence check, it is recommended to test mutations individually (do not set the ``maxbatchsize`` parameter).

.. because why?

Call the ``create_mutated.sh`` script with -c to obtain a module where you can turn on and off the mutation using ``mutsel``:

.. code-block:: text

	bash $SCRIPTS/create_mutated.sh -c

In general, using this way of exporting the original and mutated simultaneously makes it easier to implement the miter circuit as you do not have to worry about conflicting module names. When using SBY, it additionally offers the advantage that you can use the ``fmcombine`` optimization pass that analyzes the module and combines any logic that is not in the fanout cone of the mutation, and hence identical between the two module instances.

Running the equivalence check
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Set up a script that verifies these assertions using a formal tool. For example, with SBY, you would first create a project file ``test_eq.sby``:

.. code-block:: text

	[options]
	mode bmc
	depth 10
	expect pass,fail

	[engines]
	smtbmc boolector

	[script]
	read_verilog -sv miter.sv
	read_verilog mutated.v
	prep -top miter
	fmcombine miter ref uut
	flatten
	opt -fast

	[files]
	miter.sv
	mutated.v

If using BMC, make sure to set the depth sufficiently high to fully explore any pipelines or state machines in the module. Also note the use of ``fmcombine`` which optimizes the model to remove redundant logic between the two modules.

Then you can use this to run the equivalence check with SBY:

.. code-block:: text

	ln -s ../../test_eq.sv ../../test_eq.sby .
	sby -f test_eq.sby

Reporting equivalence
~~~~~~~~~~~~~~~~~~~~~

Finally, write the result of the equivalence check to the file ``output.txt``, preceded by ``1``:

.. code-block:: text

	gawk "{ print 1, \$1; }" test_eq/status >> output.txt
