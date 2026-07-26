# Project history

The university assignment was delivered in three snapshots:

| Stage | Source snapshot | Scope |
|---|---|---|
| A | `11ea54af63029b237a78e65606db9b9c437e8638` | Initial sequential renderer and parsers |
| B | `afb938a78d0cc7612c35bacf3da36c14cd68fa47` | Complete sequential AoS and SoA renderers, tests and sample scenes |
| PAR | `6302217171cc12b790e73fb14a7b6b060668cf99` | oneTBB configuration and blocked 2D parallel rendering |

This portfolio edition uses B as its correctness baseline and integrates the
parallel work from PAR. The integration fixes missing source references,
keeps sample averaging and gamma correction outside the sampling loop, uses a
thread-local BVH traversal stack, and gives each pixel deterministic random
number streams. These changes make the output independent of the selected TBB
thread count and partitioner.

The original university repositories are source-only references and have not
been modified.
