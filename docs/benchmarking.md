# Benchmarking

Build the release targets and run the standard matrix:

```bash
cmake --preset release
cmake --build --preset release
python3 scripts/benchmark.py --build-dir out/build/release
```

The script records render-only wall-clock time reported by each executable.
It tests sequential AoS, sequential SoA, and the oneTBB SoA renderer at
multiple thread counts. Raw results are written to
`out/benchmarks/results.csv`.

For meaningful comparisons:

- close CPU-intensive applications;
- use a release build;
- run at least five repetitions;
- report the CPU model, compiler and operating system;
- compare medians rather than a single run;
- do not compare results collected on different machines.

The `Benchmark` GitHub Actions workflow can also be launched manually. It
uploads its CSV as a workflow artifact, providing a repeatable hosted-runner
baseline.
