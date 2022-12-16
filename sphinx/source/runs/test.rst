==========
Test - AIR
==========

-------
Initial
-------

.. graphviz:: ../test-dump/air.initial.tree.dot

------------------
After TRS simplify
------------------

.. graphviz:: ../test-dump/air.trs-simplify.tree.dot

------------
After DCE #1
------------

.. graphviz:: ../test-dump/air.dce-1.tree.dot

--------------------------
After reaching definitions
--------------------------

.. graphviz:: ../test-dump/air.reaching-defs.tree.dot

--------------
After lex vars
--------------

.. graphviz:: ../test-dump/air.lex-vars.tree.dot

------------------------
After propagate types #1
------------------------

.. graphviz:: ../test-dump/air.propagate-types-1.tree.dot

------------------------
After act rec conversion
------------------------

.. graphviz:: ../test-dump/air.act-rec-convert.tree.dot

--------------
After simplify
--------------

.. graphviz:: ../test-dump/air.simplify.tree.dot

------------------------
After propagate barriers
------------------------

.. graphviz:: ../test-dump/air.propagate-barriers.tree.dot

-----------------
After reassociate
-----------------

.. graphviz:: ../test-dump/air.reassociate.tree.dot

--------------------
After propagate reps
--------------------

.. graphviz:: ../test-dump/air.propagate-reps.tree.dot

------------------
After convert reps
------------------

.. graphviz:: ../test-dump/air.convert-reps.tree.dot

------------------------
After propagate types #2
------------------------

.. graphviz:: ../test-dump/air.propagate-types-2.tree.dot

=====================
Test - CIR (ABI call)
=====================

-------
Initial
-------

.. graphviz:: ../test-dump/cir.abi_call.initial.ssa.dot

----------
Number Ops
----------

.. graphviz:: ../test-dump/cir.abi_call.number-ops.ssa.dot

.. graphviz:: ../test-dump/cir.abi_call.number-ops.live_ranges.dot

-----------
Live ranges
-----------

.. graphviz:: ../test-dump/cir.abi_call.live-ranges.ssa.dot

.. graphviz:: ../test-dump/cir.abi_call.live-ranges.live_ranges.dot

-------------------
After Allocate regs
-------------------

.. graphviz:: ../test-dump/cir.abi_call.allocate-regs.ssa.dot

.. graphviz:: ../test-dump/cir.abi_call.allocate-regs.live_ranges.dot

------------------
After Resolve Phis
------------------

.. graphviz:: ../test-dump/cir.abi_call.resolve-phis.ssa.dot

.. graphviz:: ../test-dump/cir.abi_call.resolve-phis.live_ranges.dot

----------------------
After Remove empty BBs
----------------------

.. graphviz:: ../test-dump/cir.abi_call.remove-empty-bbs.ssa.dot

======================
Test - CIR (Full call)
======================

-------
Initial
-------

.. graphviz:: ../test-dump/cir.full_call.initial.ssa.dot

--------------------
After full call args
--------------------

.. graphviz:: ../test-dump/cir.full_call.full-call-args.ssa.dot

-----------
Live ranges
-----------

.. graphviz:: ../test-dump/cir.full_call.live-ranges.live_ranges.dot

-------------------
After Allocate regs
-------------------

.. graphviz:: ../test-dump/cir.full_call.allocate-regs.ssa.dot

.. graphviz:: ../test-dump/cir.full_call.allocate-regs.live_ranges.dot

------------------
After Resolve Phis
------------------

.. graphviz:: ../test-dump/cir.full_call.resolve-phis.ssa.dot

.. graphviz:: ../test-dump/cir.full_call.resolve-phis.live_ranges.dot

----------------------
After Remove empty BBs
----------------------

.. graphviz:: ../test-dump/cir.full_call.remove-empty-bbs.ssa.dot
