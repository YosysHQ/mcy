Configuration File Format
=========================

``mcy`` relies on a configuration file named ``config.mcy``. There are several sections in an mcy configuration file:

``[options]``
-------------

This section contains various configuration options for the mutation generation as well as the tags used.

``size <num>``
	The number of mutations to be generated.

``tags <tagname>..``
	The tags used in the ``[logic]`` and ``[report]`` sections.

``seed``
	Optional. Random number generator seed, can be set for reproducible mutation lists. (If you wish to know the randomly chosen seed of an existing set of mutations, it can be found in ``database/mutations.ys``.)

``select <selection>``
	Optional. Selection of a subset of the design to restrict mutations to. See `yosys -h select <http://www.clifford.at/yosys/cmd_select.html>`_ for a description for the selection pattern format.

	.. note:: The ``select`` keyword here is not the Yosys ``select`` command. The argument ``<selection>`` is used as the optional selection argument to the Yosys ``mutate`` command. While the selection pattern format is identical, you cannot use select subcommands such as ``-module``.

Mutation generation options: ``mcy`` attempts to distribute mutations into all parts of the design. The documentation section :ref:`mutgen` describes the mutation generation algorithm, and how these values affect it.

``weight_cover``
	Optional. Weight for source location coverage. See :ref:`mutgen` for details. Default: 500

``weight_pq_w weight_pq_b weight_pq_c weight_pq_s``
	Optional. Weights for the per-design wire/bit/cell/src queues.
	See :ref:`mutgen` for details. Default: 100

``weight_pq_mw weight_pq_mb weight_pq_mc weight_pq_ms``
	Optional. Weights for the per-module wire/bit/cell/src queues.
	See :ref:`mutgen` for details. Default: 100

``pick_cover_prcnt``
	Optional. Chance that source location coverage influences which queue item is picked. See :ref:`mutgen` for details. Default: 80

``[setup]``
-----------

Optional. This section can contain a bash script to be executed at the beginning of ``mcy init``, which is useful for various setup tasks that only need to be done once for use in the design and/or tests. This script is executed in the base project directory (where ``config.mcy`` is located).

``[script]``
------------

This section contains the Yosys script to be used to prepare the design to be mutated.
Read in the necessary files with calls to ``read`` (one line per file), then call ``prep -top <top_module>`` to elaborate the design. (See `yosys -h read <http://www.clifford.at/yosys/cmd_read.html>`_ for the options to the ``read`` command.)

``[files]``
-----------

This section lists the files used by the design that should be added to the database. All files read in the above script should be listed here.

``[logic]``
-----------

This section describes how the mutations should be tagged based on the results of one or more tests.
It contains a python script making use of the predefined functions ``result(testname)`` and ``tag(tagname)``.

Valid arguments to ``result(testname)`` are names of tests defined in a ``[test testname]`` section.
Its return value is the return value of the test, as written to ``output.txt`` in the test script execution. Both values are strings, so e.g. for a test defined as ``[test sim]``, the function should be called as ``result("sim")`` and may return ``"PASS"`` or ``"FAIL"``.
Calling this function causes the test in question to be scheduled. This means that it is possible to execute a test conditionally on another result.

Valid arguments to ``tag(tagname)`` are tags defined in the ``[options]`` section under ``tags``, again as strings. Calling this function causes the mutation to be tagged with this tag.

A common combination of using these functions is the following:

.. code-block:: text

	[logic]
	if (result("sim") == "PASS"):
		if (result("eq") == "PASS"):
			tag("NOCHANGE")
		else:
			tag("UNCOVERED")
	else:
		tag("COVERED")

This causes the test ``eq`` to only be run if the test ``sim`` passes.

As this section can contain arbitrary python, the logic can also be defined in a separate file, and used with ``import external_logic.py``.

``[report]``
------------

This section contains the script to print the results. It can make use of the predefined function ``tags(tagname)``, which returns the number of mutations tagged with the given tag.

Example:

.. code-block:: text

	[report]
	if tags("COVERED")+tags("UNCOVERED"):
	    print("Coverage: %.2f%%" % (100.0*tags("COVERED")/(tags("COVERED")+tags("UNCOVERED"))))

``[test <testname>]``
---------------------

This section defines a test. Details about how to set up tests can be found in :ref:`testsetup`.

``expect <result>..``
	The expected return values of the test in question. (By convention, usually includes ``PASS`` and ``FAIL``, although this is not mandatory). A return value not included in this list will cause the mcy run to be aborted immediately.

``run <command>``
	How to run the test. ``<command>`` is executed in a temporary subdirectory created for the task, ``tasks/<uuid>/``. ``mcy`` creates a file ``input.txt`` with a numbered list of mutations to be tested, and expects the results of the test to be written to ``output.txt`` after execution of ``<command>`` with the same number identifying the mutation.

``maxbatchsize <X>``
	How many mutations to include in a single task. Default is 1. Increasing this number will cause ``mcy`` to add up to ``<X>`` lines to ``input.txt`` for each task.
