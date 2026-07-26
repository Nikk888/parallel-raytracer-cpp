#!/usr/bin/env python3
"""Run reproducible AoS, SoA, and oneTBB renderer benchmarks."""

from __future__ import annotations

import argparse
import csv
import re
import subprocess
import tempfile
from datetime import datetime, timezone
from pathlib import Path


TIME_PATTERN = re.compile(r"TIME_MS=(\d+)")


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    parser.add_argument("--build-dir", type=Path, default=Path("out/build/release"))
    parser.add_argument(
        "--config", type=Path, default=Path("examples/benchmark_config.txt")
    )
    parser.add_argument("--scene", type=Path, default=Path("examples/scene4.txt"))
    parser.add_argument("--results", type=Path, default=Path("out/benchmarks/results.csv"))
    parser.add_argument("--repeats", type=int, default=5)
    parser.add_argument("--threads", type=int, nargs="+", default=[1, 2, 4, 8])
    return parser.parse_args()


def run_renderer(
    executable: Path, config: Path, scene: Path, output: Path
) -> int:
    completed = subprocess.run(
        [str(executable), str(config), str(scene), str(output)],
        check=True,
        capture_output=True,
        text=True,
    )
    match = TIME_PATTERN.search(completed.stdout)
    if match is None:
        raise RuntimeError(f"{executable} did not report TIME_MS")
    return int(match.group(1))


def parallel_config(source: Path, threads: int, destination: Path) -> None:
    contents = source.read_text(encoding="utf-8")
    destination.write_text(
        contents.rstrip()
        + "\n"
        + f"tbb_threads: {threads}\n"
        + "tbb_partitioner: static\n"
        + "tbb_grain_x: 16\n"
        + "tbb_grain_y: 4\n",
        encoding="utf-8",
    )


def main() -> None:
    args = parse_args()
    if args.repeats < 1 or any(value < 1 for value in args.threads):
        raise SystemExit("repeats and thread counts must be positive")

    executables = {
        "aos": args.build_dir / "aos" / "render-aos",
        "soa": args.build_dir / "soa" / "render-soa",
        "par": args.build_dir / "par" / "render-par",
    }
    missing = [str(path) for path in executables.values() if not path.is_file()]
    if missing:
        raise SystemExit("Missing executables:\n" + "\n".join(missing))

    args.results.parent.mkdir(parents=True, exist_ok=True)
    output_dir = args.results.parent / "images"
    output_dir.mkdir(parents=True, exist_ok=True)
    timestamp = datetime.now(timezone.utc).isoformat()
    rows: list[dict[str, str | int]] = []

    for variant in ("aos", "soa"):
        for repeat in range(1, args.repeats + 1):
            output = output_dir / f"{variant}-{repeat}.ppm"
            elapsed = run_renderer(
                executables[variant], args.config, args.scene, output
            )
            rows.append(
                {
                    "timestamp_utc": timestamp,
                    "variant": variant,
                    "threads": 1,
                    "repeat": repeat,
                    "time_ms": elapsed,
                }
            )

    with tempfile.TemporaryDirectory(prefix="renderer-benchmark-") as directory:
        temporary = Path(directory)
        for threads in args.threads:
            config = temporary / f"parallel-{threads}.txt"
            parallel_config(args.config, threads, config)
            for repeat in range(1, args.repeats + 1):
                output = output_dir / f"par-{threads}-{repeat}.ppm"
                elapsed = run_renderer(
                    executables["par"], config, args.scene, output
                )
                rows.append(
                    {
                        "timestamp_utc": timestamp,
                        "variant": "par",
                        "threads": threads,
                        "repeat": repeat,
                        "time_ms": elapsed,
                    }
                )

    with args.results.open("w", newline="", encoding="utf-8") as handle:
        writer = csv.DictWriter(
            handle,
            fieldnames=["timestamp_utc", "variant", "threads", "repeat", "time_ms"],
        )
        writer.writeheader()
        writer.writerows(rows)

    print(f"Wrote {len(rows)} measurements to {args.results}")


if __name__ == "__main__":
    main()
