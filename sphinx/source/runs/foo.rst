=========
Foo - AIR
=========

-------
Initial
-------

.. graphviz:: ../dump/air.initial.tree.dot

------------------
After TRS simplify
------------------

.. graphviz:: ../dump/air.trs-simplify.tree.dot

------------
After DCE #1
------------

.. graphviz:: ../dump/air.dce-1.tree.dot

--------------------------
After reaching definitions
--------------------------

.. graphviz:: ../dump/air.reaching-defs.tree.dot

--------------
After lex vars
--------------

.. graphviz:: ../dump/air.lex-vars.tree.dot

------------------------
After propagate types #1
------------------------

.. graphviz:: ../dump/air.propagate-types-1.tree.dot

------------------------
After act rec conversion
------------------------

.. graphviz:: ../dump/air.act-rec-convert.tree.dot

--------------
After simplify
--------------

.. graphviz:: ../dump/air.simplify.tree.dot

------------------------
After propagate barriers
------------------------

.. graphviz:: ../dump/air.propagate-barriers.tree.dot

-----------------
After reassociate
-----------------

.. graphviz:: ../dump/air.reassociate.tree.dot

--------------------
After propagate reps
--------------------

.. graphviz:: ../dump/air.propagate-reps.tree.dot

------------------
After convert reps
------------------

.. graphviz:: ../dump/air.convert-reps.tree.dot

------------------------
After propagate types #2
------------------------

.. graphviz:: ../dump/air.propagate-types-2.tree.dot

====================
Foo - CIR (ABI call)
====================

-------
Initial
-------

.. graphviz:: ../dump/cir.abi_call.initial.ssa.dot

----------
Number Ops
----------

.. graphviz:: ../dump/cir.abi_call.number-ops.ssa.dot

.. graphviz:: ../dump/cir.abi_call.number-ops.live_ranges.dot

-----------
Live ranges
-----------

.. graphviz:: ../dump/cir.abi_call.live-ranges.ssa.dot

.. graphviz:: ../dump/cir.abi_call.live-ranges.live_ranges.dot

-------------------
After Allocate regs
-------------------

.. graphviz:: ../dump/cir.abi_call.allocate-regs.ssa.dot

.. graphviz:: ../dump/cir.abi_call.allocate-regs.live_ranges.dot

------------------
After Resolve Phis
------------------

.. graphviz:: ../dump/cir.abi_call.resolve-phis.ssa.dot

.. graphviz:: ../dump/cir.abi_call.resolve-phis.live_ranges.dot

----------------------
After Remove empty BBs
----------------------

.. graphviz:: ../dump/cir.abi_call.remove-empty-bbs.ssa.dot

=====================
Foo - CIR (Full call)
=====================

-------
Initial
-------

.. graphviz:: ../dump/cir.full_call.initial.ssa.dot

--------------------
After full call args
--------------------

.. graphviz:: ../dump/cir.full_call.full-call-args.ssa.dot

-----------
Live ranges
-----------

.. graphviz:: ../dump/cir.full_call.live-ranges.live_ranges.dot

-------------------
After Allocate regs
-------------------

.. graphviz:: ../dump/cir.full_call.allocate-regs.ssa.dot

.. graphviz:: ../dump/cir.full_call.allocate-regs.live_ranges.dot

------------------
After Resolve Phis
------------------

.. graphviz:: ../dump/cir.full_call.resolve-phis.ssa.dot

.. graphviz:: ../dump/cir.full_call.resolve-phis.live_ranges.dot

----------------------
After Remove empty BBs
----------------------

.. graphviz:: ../dump/cir.full_call.remove-empty-bbs.ssa.dot
