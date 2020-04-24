Command Reference
=================

``mcy`` provides the following commands:

.. code-block:: text

	> mcy help

	Usage:
		mcy [--trace] init
		mcy [--trace] reset
		mcy [--trace] status
		mcy [--trace] list [--details] [<id_or_tag>..]
		mcy [--trace] run [-jN] [--reset] [<id>..]
		mcy [--trace] task [-v] [-k] <test> <id_or_tag>..
		mcy [--trace] source [-e <encoding>] <filename> [<filename>]
		mcy [--trace] lcov <filename>
		mcy [--trace] dash [<source_dir>]
		mcy [--trace] gui [--src <source_dir>]
		mcy [--trace] purge

All commands require the project configuration file ``config.mcy`` to be present in the current directory.

mcy init
	This command initializes the mcy database. It prepares the design using the script from the ``[script]`` section in ``config.mcy``, and generates a list of mutations conforming to the settings in the ``[options]`` section. It queues all mutations to be tested when ``mcy run`` is called.
	The command fails if the ``database`` directory exists. Run ``mcy purge`` to delete this directory if it is present.

mcy reset
	This command will reset various state. If the ``size`` parameter in the section ``[options]`` of ``config.mcy`` was increased, it will create additional mutations. It will re-run the tagging logic of the ``[logic]`` section and re-tag all mutations for which results are cached in the database. It queues the mutations for which results are not available to be tested when ``mcy run`` is called. It will also delete an existing ``tasks`` directory.

mcy status
	This command prints the status of the project. It will indicate the number of cached results and queued tests. If some results are available, it will also report the results in the format specified in the ``[report]`` section of ``config.mcy``.
	The same status is also printed at the end of the commands ``init``, ``reset``, and ``run``.

mcy list [--details] [<id_or_tag>..]
	This command prints the list of selected mutations and the tags applied to them. If the optional selection argument ``<id_or_tag>`` is not present, all mutations are listed. There can be multiple selection arguments, in which case mutations matching any of the IDs or tags are listed. If ``--details`` is passed, it will additionally print the mutation command and the results cached in the database.

mcy run [-jN] [--reset] [<id>..]
	This command executes the tests in the queue and any tests subsequently queued based on the results (for conditionally executed tests). The optional argument ``-j N`` allows up to ``N`` tasks to be executed in parallel. If ``--reset`` is passed, ``mcy reset`` will run first (potentially creating additional mutations or queueing more tasks). The optional selection argument ``<id>``, of which there can be multiple, restricts mcy to run only tests on the matching mutation(s). (Tests for which results are available will not be re-run.)

mcy task [-v] [-k] <test> <id_or_tag>..
	This command runs the test ``<test>`` on the mutations matching the ID or tag ``<id_or_tag>``, of which there can be multiple. The test is executed even if the result is cached in the database. If the ``-v`` flag is passed, the output of the task execution is printed to stdout instead of the file ``tasks/<uuid>/logfile.txt``. If ``-k`` is passed, the temporary task execution directory ``tasks/<uuid>`` is not deleted when the task finishes.

mcy source [-e <encoding>] <filename> [<filename>]
	This command reprints the source file(s) <filename>, with annotations on the left side margin for each line of code with the number of mutations tagged COVERED or UNCOVERED (the name of the tags used is hardcoded). The number of COVERED mutations in displayed as a positive number, whereas UNCOVERED mutations are shown as negative numbers, similar to what is shown in ``mcy gui``. Source files are printed from database cache, which is written when ``mcy init`` is called, so the version displayed is always the one the mutations were applied to. The optional ``-e`` parameter allows specifying the file encoding. (Python's `standard encodings <https://docs.python.org/3/library/codecs.html#standard-encodings>`_ are supported, default is utf8.)

mcy lcov <filename>
	This command prints coverage information in lcov format (??)

mcy dash
	This command launches the dashboard server. Navigate to the address shown to access the dashboard where you can monitor the ``mcy`` status and download the mcy database for viewing with ``mcy-gui``. This is intended to be used when running ``mcy`` on a remote server or as part of CI.

mcy gui
	Launch the graphical explorer. The mcy gui allows navigating the list of mutations by source location, mutation id, or tag, and is the easiest way to access associated information.
	If you have downloaded a database from the mcy dashboard, invoke it directly as ``mcy-gui <db_filename>``, as the mcy wrapper script will abort if ``config.mcy`` is not present.

mcy purge
	This command removes all files produced by ``mcy``, i.e. it deletes the ``database`` and ``tasks`` directories. Run this command before running ``mcy init``.

The optional ``--trace`` argument to ``mcy`` is used for debugging and prints all queries performed on the database during execution to stdout.
