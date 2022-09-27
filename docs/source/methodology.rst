Methodology
===========

This section describes the principles underlying MCY which should be kept in mind during project setup.

The idea behind MCY is to test how good your test suite is at detecting errors in your design, by deliberately introducing modifications (small "mutations" that change the value of a single bit in the design) and checking if they are caught. However, it is not guaranteed that a mutation will break the design: if e.g. the value of the bit only changes on cycles where an associated ``valid`` signal is low, the design still functions as intended and the test suite would be correct in passing it.

.. topic:: Prerequisite: your test suite is passing on the original design

	It is not possible to measure mutation coverage for a failing design. If your original design is already broken, breaking it further by introducing a mutation cannot make a detectable change to the test status.

Many coverage tools would simply make you pick an arbitrary "target coverage rate" lower than 100% based on your intuition or experience, to account for those cases where the mutation does not make a relevant difference. With MCY, you will set up a formal equivalence check that serves as the "truth" about whether a mutation affects the functioning of your design. If the equivalence check can find a valid combination of inputs for which the output of the mutated design differs from the original, but the test suite passes the design anyway, then the test suite is not set up to catch this type of incorrect behavior. The goal then becomes to reach 100% coverage.

.. topic:: Equivalence check

	Setting up the formal equivalence check requires a little upfront effort, as you will need to specify which variations in input and output signals are a functional difference according to your design specification, but this is generally not too hard if your interfaces are well specified. In contrast to full formal verification, it is much easier to say that, for example, if the mutated design outputs ``5`` where the original returns ``3``, at least one of the values must be wrong, than it is to write formal properties that calculate that the result *should* be ``42`` based on inputs that might have been entered over several previous cycles.

However, for the core MCY engine there is no difference in how to run this test compared to the other tests in your test suite, so there is no special handling baked into the configuration file format to guide you in setting your project up correctly. If a single author is writing both the test suite and the equivalence test, the distinction between the two may become blurred. When setting up the equivalence test and especially when writing the ``[logic]`` section of the configuration, it is crucial that you keep this core idea at the forefront of your mind:

.. topic:: Fundamental Principle

	The equivalence check is the reference standard against which your test suite is compared.
	It will tell you whether there is a relevant behavior change to be found in a mutated design, and your test suite is judged by whether it accurately detects this change or not.

.. note::

	This is **not** saying that a design that passes equivalence check is correct, or a design that fails equivalence is buggy. Since most likely the original design is not perfectly bug-free, being equivalent to it cannot tell you anything about correctness.

	What this serves to determine is whether **your testbench is capable of detecting the kind of changes in behavior** that the mutation introduced.

When starting an MCY project, it is recommended to start by using this "naive" logic directly, and keeping the number of mutations small. As you gain confidence that the setup is working, you can progressively add optimizations that let you run larger numbers of mutations.

The ``bitcnt`` example shows an implementation of the naive logic:

.. code-block:: text

	sim_okay = result("test_sim") == "PASS"
	eq_okay = result("test_eq") == "PASS"

	if sim_okay and not eq_okay:
	    tag("UNCOVERED")
	elif not sim_okay and not eq_okay:
	    tag("COVERED")
	elif sim_okay and eq_okay:
	    tag("NOCHANGE")
	elif not tb_okay and eq_okay:
	    tag("EQGAP")
	else:
	    assert 0

This unconditionally runs both the test suite (which consists of a single testbench ``test_sim``), and the equivalence check ``test_eq``. The first three cases are the expected ones: if the equivalence check says there is a relevant difference in behavior between the original and mutated designs, then if the testbench catches it the mutation is covered, and if it fails to catch it it is not covered. If the equivalence check and the testbench both agree that the original and mutated designs behave the same way, the mutation is not useful for evaluating testbench quality and can be ignored.

Keep an eye out for that fourth category! In a correctly set up project, this should never happen. If the equivalence check finds that the mutation does not affect the behavior, but the testbench finds a bug, one of them is wrong. Either the equivalence check is failing to find relevant behavioral changes, or the testbench is failing compliant designs. Investigate these cases thoroughly. If the behavior with the mutation in question is OK, adjust the testbench to pass it. If the behavior is not OK, modify the equivalence check to detect the problematic change. Only proceed to optimize the logic once you are not observing any cases of the fourth type.

For performance reasons, you will often want to end up at a logically equivalent but very different flow in the ``[logic]`` section, and this can be a source of confusion when looking at more complex, optimized examples. There are two assumptions that can be used to avoid running some of the tests, and still arrive at the same result:

.. topic:: Assumption 1: The equivalence check is correct.

	If the equivalence check fails, the mutation changes the functioning of the design, if it passes, there is no functional difference between the original and the mutated design.

In particular, this means that if the equivalence check passes, there is no need to run any of the test suite on this mutation. Since the mutation does not have any relevant effect, it cannot tell us anything about the test suite's ability to detect bugs.

.. topic:: Assumption 2: The test suite will not return false failures for a working design.

	Ensure that your testbenches are tolerant of small changes in behavior that are not relevant to correctness, such as e.g. the values of output bits that are unused in certain modes.

With this assumption, if any test in the suite returns FAIL, there is no need to run either other tests from the test suite or the equivalence check: the mutation is covered.
This allows you to run short unit tests first, and determine the result quickly for the more "obviously buggy" mutations.

Any case of ``EQGAP`` violates at least one of these assumptions. This is why we recommend testing for this case in the beginning, to gain confidence that the assumptions hold. It is also possible to use ``rng()`` to check that the assumption holds only in a random subset of mutations, to balance execution time concerns with confidence in the test's correctness.

In general, to get the best performance, always run the shortest test first. If you have unit tests, run these first, and integration tests after, in increasing order of runtime. The equivalence test will often be one of the longer-running tests, especially for the worst-case where the designs are equivalent.

.. topic:: Example

	If your test suite contains three testbenches of increasing run length: a unit test ``test_unit``, an integration test ``test_sys`` and a hardware-in-the-loop test ``test_hw``, and the equivalence check ``test_eq`` generally takes longer than ``test_sys`` but less time than ``test_hw``, then applying the two assumptions to finish early whenever possible would lead to the following logic:

	.. code-block:: text

		if result("test_unit") == "FAIL":
			tag("COVERED")
			return

		if result("test_sys") == "FAIL":
			tag("COVERED")
			return

		if result("test_eq") == "PASS":
			tag("NOCHANGE")
			return

		if result("test_hw") == "FAIL":
			tag("COVERED")
			return

		tag("UNCOVERED")

	As you can see, the underlying reasoning is no longer obvious from this code. Always keep the fundamental principle and the two assumptions in mind while working with MCY!
