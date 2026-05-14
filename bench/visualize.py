import re
from pathlib import Path

import pandas as pd
import matplotlib.pyplot as plt


INPUT_DIR = Path("dump")
OUT_DIR = Path("figures")
OUT_DIR.mkdir(exist_ok=True)

REMOVE_EMPTY_LOOP = False
MAKE_LOG_GRAPHS = True


def read_results(filename: str) -> pd.DataFrame:
    path = INPUT_DIR / filename
    if not path.exists():
        raise FileNotFoundError(f"Missing input file: {path}")

    df = pd.read_csv(path)

    for col in [
        "implementation",
        "benchmark",
        "opt-level",
        "direct-linking",
        "int-cache-range",
    ]:
        df[col] = df[col].astype(str)

    df["direct-linking"] = df["direct-linking"].str.lower()

    for col in ["avg-ms", "std-dev-ms", "std-dev-pct", "slowdown-vs-clojure"]:
        df[col] = pd.to_numeric(df[col], errors="coerce")

    return df


def tinyclojure_rows(df: pd.DataFrame) -> pd.DataFrame:
    tc = df[df["implementation"] == "TinyClojure"].copy()

    if REMOVE_EMPTY_LOOP:
        tc = tc[~tc["benchmark"].str.contains("empty", case=False, na=False)]

    if tc.empty:
        raise RuntimeError("No TinyClojure rows left after filtering.")

    return tc


def cache_sort_key(value: str):
    if value == "off":
        return (-1, 0)

    match = re.fullmatch(r"(-?\d+):(-?\d+)", value)
    if match:
        lo = int(match.group(1))
        hi = int(match.group(2))
        return (0, abs(lo) + abs(hi))

    return (1, value)


def discovered_order(df: pd.DataFrame, column: str, preferred=None):
    values = [v for v in df[column].dropna().astype(str).unique().tolist()]

    if preferred is not None:
        preferred_present = [v for v in preferred if v in values]
        remaining = [v for v in values if v not in preferred_present]
        return preferred_present + remaining

    return values


def plot_grouped_bars(
        df: pd.DataFrame,
        group_column: str,
        group_order,
        ylabel: str,
        xlabel: str,
        legend_title: str,
        output_name: str,
):
    if df.empty:
        raise RuntimeError(f"No data for graph {output_name}")

    df = df.copy()
    df[group_column] = pd.Categorical(
        df[group_column],
        categories=group_order,
        ordered=True,
    )

    pivot_avg = df.pivot_table(
        index="benchmark",
        columns=group_column,
        values="avg-ms",
        aggfunc="first",
        observed=False,
    ).reindex(columns=group_order)

    pivot_std = df.pivot_table(
        index="benchmark",
        columns=group_column,
        values="std-dev-ms",
        aggfunc="first",
        observed=False,
    ).reindex(columns=group_order)

    # Drop columns that are entirely missing.
    non_empty_columns = pivot_avg.columns[~pivot_avg.isna().all()]
    pivot_avg = pivot_avg[non_empty_columns]
    pivot_std = pivot_std[non_empty_columns]

    if pivot_avg.empty:
        raise RuntimeError(f"Pivot table for {output_name} is empty.")

    ax = pivot_avg.plot(
        kind="bar",
        figsize=(8, 4),
        yerr=pivot_std,
        capsize=4,
    )
    ax.set_ylabel(ylabel)
    ax.set_xlabel(xlabel)
    ax.legend(title=legend_title)
    plt.xticks(rotation=20, ha="right")
    plt.tight_layout()
    plt.savefig(OUT_DIR / output_name)
    plt.close()

    if MAKE_LOG_GRAPHS:
        log_name = output_name.replace(".pdf", "-log.pdf")

        ax = pivot_avg.plot(
            kind="bar",
            figsize=(8, 4),
            yerr=pivot_std,
            capsize=4,
            logy=True,
        )
        ax.set_ylabel(f"{ylabel}, log scale")
        ax.set_xlabel(xlabel)
        ax.legend(title=legend_title)
        plt.xticks(rotation=20, ha="right")
        plt.tight_layout()
        plt.savefig(OUT_DIR / log_name)
        plt.close()


# ------------------------------------------------------------
# Graph 1: integer cache range
#
# Intended benchmark configuration:
#   direct linking: enabled
#   LLVM optimizations: enabled
#   varied option: integer cache range
# ------------------------------------------------------------

cache_df = tinyclojure_rows(read_results("integer-caching.csv"))

cache_order = sorted(
    cache_df["int-cache-range"].dropna().astype(str).unique().tolist(),
    key=cache_sort_key,
)

plot_grouped_bars(
    df=cache_df,
    group_column="int-cache-range",
    group_order=cache_order,
    ylabel="Average time [ms]",
    xlabel="Benchmark",
    legend_title="Integer cache",
    output_name="benchmark-integer-cache-effect.pdf",
)


# ------------------------------------------------------------
# Graph 2: LLVM optimization level
#
# Intended benchmark configuration:
#   integer cache: default range
#   direct linking: enabled
#   varied option: LLVM optimization level
# ------------------------------------------------------------

llvm_df = tinyclojure_rows(read_results("llvm-optimizations.csv"))

opt_order = discovered_order(
    llvm_df,
    "opt-level",
    preferred=["O0", "O1", "O2", "O3"],
)

plot_grouped_bars(
    df=llvm_df,
    group_column="opt-level",
    group_order=opt_order,
    ylabel="Average time [ms]",
    xlabel="Benchmark",
    legend_title="LLVM optimization level",
    output_name="benchmark-llvm-optimization-effect.pdf",
)


# ------------------------------------------------------------
# Graph 3: direct linking
#
# Intended benchmark configuration:
#   integer cache: default range
#   LLVM optimizations: enabled
#   varied option: direct linking
# ------------------------------------------------------------

direct_df = tinyclojure_rows(read_results("direct-linking.csv"))

direct_order = discovered_order(
    direct_df,
    "direct-linking",
    preferred=["false", "true"],
)

plot_grouped_bars(
    df=direct_df,
    group_column="direct-linking",
    group_order=direct_order,
    ylabel="Average time [ms]",
    xlabel="Benchmark",
    legend_title="Direct linking",
    output_name="benchmark-direct-linking-effect.pdf",
)


print(f"Wrote figures to {OUT_DIR.resolve()}")