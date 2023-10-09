.. _mutate:

Mutation export options
=======================

The first step in any test script is to obtain the mutated module to run the test on.

There are three configurations that are commonly used:

1. Permanently enabled single mutation. Applying a single mutation without any control options results in a module with the same interface as the original module that can be substituted in a testbench without requiring any modifications. This is generally the most straightforward way to run the testsuite that is being evaluated.

2. Selectable single mutation. Applying a single mutation with a control signal results in a module with one additional input, ``mutsel``. The mutation will be enabled if ``mutsel==1`` and disabled if ``mutsel==0``. Generally used in the equivalence test, where the same module can be instantiated twice with different ``mutsel`` inputs. This avoids problems with module name collision between the original and mutated version, and allows the use of ``fmcombine`` which can speed up the formal property solver.

3. Multiple selectable mutations. Multiple mutate commands are applied to a module simultaneously. The resulting module will have an additional ``mutsel`` input and can exhibit a different mutation for each value of ``mutsel``. This is useful if the testbench being evaluated has large setup costs, e.g. for compilation. However, it requires modifying the testbench to drive the ``mutsel`` input from an argument passed at execution time.

By default, each task in MCY will only test a single mutation. To enable multiple mutations to be tested in a single task, set the ``maxbatchsize`` parameter in the corresponding ``[test]`` section in ``config.mcy`` to a value larger than 1 and small enough to be representable with the number of bits of the ``mutsel`` signal (set in the ``mutate -ctrl <width> <value>`` option).

The ``create_mutated.sh`` script
--------------------------------

If your test falls within the common use cases, you can use the script ``create_mutated.sh`` to export the modified module. This is generally more convenient than writing your own script.

The script is made to work with the files set up by mcy in the temporary task execution directory. MCY also sets up the environment variable ``SCRIPTS`` pointing to the directory where the scripts are installed. Call it in your test script as follows:

.. code-block:: text

	bash $SCRIPTS/create_mutated.sh

By default it will produce a ``mutated.v`` with permanently enabled mutation (case 1). Pass the ``-c`` (or ``--ctrl``) option to add the ``mutsel`` input for cases 2 and 3.

The task mutation list ``input.txt``
------------------------------------

For each test, MCY will create a file named ``input.txt`` that contains lines of the format:

.. code-block:: text

	<idx> <mutate command (without -ctrl)>

The index identifies the mutation within the current batch. If ``maxbatchsize`` is not set for this task, mutations will be tested one at a time, so the file will contain a single line and ``<idx>`` will always be ``1``.
For the bitcount example, it might look like this:

.. code-block:: text

	1 mutate -mode cnot0 -module bitcnt -cell $and$bitcnt.v:53$263 -port Y -portbit 23 -ctrlbit 57 -src bitcnt.v:53

Note that the indices always go from ``1`` to ``maxbatchsize`` within a task. They are different from the mutation IDs displayed by ``mcy list`` or the GUI, do not confuse the two!

Writing a custom mutation export script
---------------------------------------

If ``create_mutated.sh`` is insufficient for your use case, you may need to write a custom script to create the mutated sources. This script should take the mutation commands from ``input.txt`` and call yosys to apply the mutations and export the mutated source to the right format. The easiest way to do this is to first write the yosys commands to a ``.ys`` script file and then run it.

The yosys script (``mutate.ys`` in this example) that you need to generate should be structured like this:

- Read the intermediary file containing the elaborated design:

  .. code-block:: text

    echo "read_verilog ../../database/design.il" > mutate.ys

- Apply the mutate command(s) found in ``input.txt``.

  If there is only one mutation, and you do not wish to create a ``mutsel`` input, enter the command as-is, just remove the leading ``1``, e.g. like this:

  .. code-block:: text

    cut -f2- -d' ' input.txt >> mutate.ys

  If you do wish to add a ``mutsel`` input to the design, you need to add the ``-ctrl`` parameter to the ``mutate`` command's arguments:

  .. code-block:: text

    while read -r idx mut; do
      echo "mutate -ctrl mutsel 8 ${idx} ${mut#* }" >> mutate.ys
    done < input.txt

  This code snippet works for one or many mutations in ``input.txt`` thanks to the ``while`` loop. If there are multiple mutations to be applied, it will add all the mutate commands to the script. Always make sure to add the control input when there are multiple mutations!

- Optionally, add any other yosys commands you wish to use to transform the design to work with your testbench. For example, if you want to run the design on hardware, you may synthesize it here:

  .. code-block:: text

    echo "synth_ice40 -top top_module" >> mutate.ys

  You can also change the name of the module if needed:

  .. code-block:: text

    echo "rename top_module mutated" >> mutate.ys

- Finally, export the design to a format that can be used by your testbench. In the example of the hardware test, this might be json:

  .. code-block:: text

    echo "write_json mutated.json" >> mutate.ys

After generating the script, execute it with yosys:

.. code-block:: text

  yosys -ql mutate.log mutate.ys

The Yosys ``mutate`` command
----------------------------

MCY's mutation capability is backed by the Yosys ``mutate`` command. This command has two functions: generating a list of mutations for a design, and applying a mutation to a design. For most users, it is not necessary to touch these internals, but understanding the inner workings of MCY may be relevant in advanced use cases.

.. _mutgen:

Mutation generation
~~~~~~~~~~~~~~~~~~~

If the ``-list <N>`` argument is given, ``mutate`` will generate a list of ``<N>`` mutations. The mutation generation algorithm tries to satisfy a variety of coverage heuristics, in an effort to avoid systematically leaving certain parts of a design untested.

The default weights should provide a decent distribution of mutations for most code bases. If you wish to fine tune the selection, you can influence the algorithm by setting any of the mutation generation options in ``config.mcy``. The decision logic works as follows:

A source location coverage scoring algorithm is picked with weight ``weight_cover``. This algorithm assigns scores to mutations based on the source locations, with locations that have fewer mutations associated so far scoring higher. (One of) the mutation(s) with top score is picked.

Alternatively, mutations can be associated with wires, wire bits, cells, and source locations. Not all mutations have all associations (e.g., some wires cannot be traced back to specific source lines), but a single mutation can appear in multiple lists. There are 8 candidate lists:

- grouped by wire
- grouped by wire bit
- grouped by cell
- grouped by source location

- grouped by module, then by wire
- grouped by module, then by wire bit
- grouped by module, then by cell
- grouped by module, then by source location

(The second set of lists ensure that even if a module is very small with respect to the number of wires/wire bits/cells/source locations, it will not be overlooked.)

One of these lists is chosen to sample a mutation from with weights ``weight_pq_w``, ``weight_pq_b``, ``weight_pq_c``, ``weight_pq_s``, ``weight_pq_mw``, ``weight_pq_mb``, ``weight_pq_mc``, ``weight_pq_ms`` respectively. Once a list is chosen, there is a ``pick_cover_prcnt`` chance that source location coverage score is used to influence which mutation is sampled, otherwise all mutations in the list are considered equally.

The output of ``mutate -list`` is a list of ``mutate`` commands that can be used to apply the generated mutations to the design. MCY generates these and stores them in the database when ``mcy init`` is run, and provides one (or several, if the ``maxbatchsize`` parameter is set for this test) in the file ``input.txt`` in the temporary folder when executing a task (via ``mcy run`` or ``mcy task``).

Applying a mutation
~~~~~~~~~~~~~~~~~~~

If the ``-mode <mode>`` argument is given, ``mutate`` will apply a mutation to the current design. Most of the other parameters (``-module, -cell, -port, -portbit, -ctrlbit, -wire, -wirebit, -src``) serve to identify the element of the design to be modified and simply need to be copied from the file ``input.txt`` provided by MCY.

There is one optional parameter of interest, and that is ``-ctrl <name> <width> <value>``. By default (without ``-ctrl``), the ``mutate`` command will replace the element to be mutated with the modified version. The resulting module is identical to the original except for this modified element.
If ``-ctrl`` is specified, the ``mutate`` command will instead add a control circuit to enable the mutation at will. It creates an additional input port to the modules, with the name ``<name>`` given in the command, and of width ``<width>``. If this input signal is set to the value ``<value>`` specified, the mutation is enabled, otherwise it is disabled. The value chosen must be non-zero, as 0 is reserved for the unmodified behaviour of the design. Multiple mutations can be added to the same design by running several ``mutate`` commands with the same ``-ctrl <name> <width>`` arguments and a different ``<value>``. This results in a design whose behaviour can be switched between different mutations by changing the value of this input signal.

The ``create_mutated.sh`` script wraps these two uses of ``mutate``, with the ``-c``/``--ctrl`` flag causing the ``-ctrl`` argument to be passed to ``mutate``.
