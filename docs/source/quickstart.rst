Getting Started
===============

Dependencies
------------

MCY requires ``python`` >= 3.5(?).

The example projects in the ``examples`` directory additionally require
`Icarus Verilog`_ and
SBY_.

.. _Icarus Verilog: http://iverilog.icarus.com/
.. _SBY: http://symbiyosys.readthedocs.io/

Installation
------------

To install mcy, run:

.. code-block:: text

	make
	sudo make install

Example
-------

To check that everything is set up correctly, you can run an example project:

.. code-block:: text

	cd examples/bitcnt
	mcy init
	mcy run -j8
