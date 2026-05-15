import re
from pathlib import Path

import pandas as pd
import matplotlib.pyplot as plt

INPUT_DIR = Path("dump")
OUT_DIR = Path("figures")
OUT_DIR.mkdir(exist_ok=True)

REMOVE_EMPTY_LOOP = False
MAKE_LOG_GRAPHS = False
GROUPED_FIGURE_HEIGHT = 5.6
DENSE_GROUPED_FIGURE_HEIGHT = 6.2
BEST_SLOWDOWN_FIGURE_HEIGHT = 5.2
AXIS_LABEL_FONTSIZE = 15
TICK_LABEL_FONTSIZE = 13
LEGEND_FONTSIZE = 11
LEGEND_TITLE_FONTSIZE = 12


def add_bar_labels(ax, suffix="", decimals=1, fontsize=7):
    for container in ax.containers:
        # Skip error-bar containers. Only bar containers have datavalues.
        if not hasattr(container, "datavalues"):
            continue

        labels = []
        for value in container.datavalues:
            if pd.isna(value):
                labels.append("")
            else:
                labels.append(f"{value:.{decimals}f}{suffix}")

        ax.bar_label(
            container,
            labels=labels,
            padding=3,
            fontsize=fontsize,
            rotation=90,
        )


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


def approximate_label_width(label: str) -> float:
    text = str(label)
    width = 0.0

    for ch in text:
        if ch in "ilI.,;:!|":
            width += 0.5
        elif ch in "mwMW":
            width += 1.4
        elif ch.isspace():
            width += 0.6
        else:
            width += 1.0

    return width


def sort_benchmarks_by_label_length(index):
    return sorted(index, key=lambda value: (approximate_label_width(value), str(value)))


def plot_grouped_bars(
        df: pd.DataFrame,
        group_column: str,
        group_order,
        ylabel: str,
        legend_title: str,
        output_name: str,
        display_names=None,
        legend_fontsize=LEGEND_FONTSIZE,
        legend_title_fontsize=LEGEND_TITLE_FONTSIZE,
        loc="best",
):
    if display_names is None:
        display_names = {
            "false": "off",
            "true": "on",
        }
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

    # Reorder benchmarks by label length to improve readability.
    # The left-most label can reserve less left padding, so shorter labels are placed there.
    benchmark_order = sort_benchmarks_by_label_length(pivot_avg.index)
    pivot_avg = pivot_avg.reindex(benchmark_order)
    pivot_std = pivot_std.reindex(benchmark_order)

    if display_names is not None:
        pivot_avg = pivot_avg.rename(columns=display_names)
        pivot_std = pivot_std.rename(columns=display_names)

    if pivot_avg.empty:
        raise RuntimeError(f"Pivot table for {output_name} is empty.")

    variant_count = len(pivot_avg.columns)

    if variant_count <= 2:
        label_fontsize = 19
        error_capsize = 6
        error_linewidth = 1.6
        x_tick_fontsize = 11
        y_tick_fontsize = 13
        axis_label_fontsize = 15
        error_capthick = 1.4
        figure_size = (8, GROUPED_FIGURE_HEIGHT)
        y_headroom = 1.35
    else:
        label_fontsize = 17
        error_capsize = 4
        error_linewidth = 1.2
        x_tick_fontsize = 15
        y_tick_fontsize = 15
        axis_label_fontsize = 17
        error_capthick = 1.0
        figure_size = (10.5, DENSE_GROUPED_FIGURE_HEIGHT)
        y_headroom = 1.2

    ax = pivot_avg.plot(
        kind="bar",
        figsize=figure_size,
        width=0.9,
        yerr=pivot_std,
        capsize=error_capsize,
        error_kw={
            "elinewidth": error_linewidth,
            "capthick": error_capthick,
        },
    )

    ax.margins(x=0.01)

    add_bar_labels(
        ax,
        decimals=1,
        fontsize=label_fontsize,
    )

    ax.set_ylabel(ylabel, fontsize=axis_label_fontsize)
    ax.set_xlabel("")
    ax.legend(
        title=legend_title,
        fontsize=legend_fontsize,
        title_fontsize=legend_title_fontsize,
        loc=loc,
    )
    plt.xticks(rotation=20, ha="right", fontsize=11)

    ymin, ymax = ax.get_ylim()
    ax.set_ylim(ymin, ymax * y_headroom)

    plt.tight_layout()
    plt.savefig(OUT_DIR / output_name, bbox_inches="tight", pad_inches=0.02)
    plt.close()

    if MAKE_LOG_GRAPHS:
        log_name = output_name.replace(".pdf", "-log.pdf")

        ax = pivot_avg.plot(
            kind="bar",
            figsize=figure_size,
            yerr=pivot_std,
            capsize=error_capsize,
            error_kw={
                "elinewidth": error_linewidth,
                "capthick": error_capthick,
            },
            logy=True,
        )

        ax.set_ylabel(f"{ylabel}, log scale")
        ax.set_xlabel("")
        ax.legend(title=legend_title)
        plt.xticks(rotation=20, ha="right", fontsize=x_tick_fontsize)
        ax.tick_params(axis="y", labelsize=y_tick_fontsize)
        plt.tight_layout()
        plt.savefig(OUT_DIR / log_name)
        plt.close()


def plot_best_slowdown(all_tc: pd.DataFrame):
    best = (
        all_tc.sort_values("slowdown-vs-clojure")
        .groupby("benchmark", as_index=False)
        .first()
    )

    benchmark_order = sort_benchmarks_by_label_length(best["benchmark"])
    best["benchmark"] = pd.Categorical(
        best["benchmark"],
        categories=benchmark_order,
        ordered=True,
    )
    best = best.sort_values("benchmark")

    if REMOVE_EMPTY_LOOP:
        best = best[~best["benchmark"].str.contains("empty", case=False, na=False)]

    if best.empty:
        raise RuntimeError("No rows available for best slowdown graph.")

    slowdown_err = (
            best["std-dev-ms"]
            / best["avg-ms"]
            * best["slowdown-vs-clojure"]
    ).fillna(0.0)

    fig, ax = plt.subplots(figsize=(8, 4.2))

    bars = ax.bar(
        best["benchmark"],
        best["slowdown-vs-clojure"],
        yerr=slowdown_err,
        capsize=6,
        error_kw={
            "elinewidth": 1.4,
            "capthick": 1.4,
        },
    )

    ax.bar_label(
        bars,
        labels=[f"{value:.1f}x" for value in best["slowdown-vs-clojure"]],
        padding=3,
        fontsize=20,
    )

    ax.set_ylabel("Slowdown vs JVM Clojure")
    ax.set_xlabel("")
    plt.xticks(rotation=20, ha="right", fontsize=13)

    ymin, ymax = ax.get_ylim()
    ax.set_ylim(ymin, ymax * 1.18)

    plt.tight_layout()
    plt.savefig(OUT_DIR / "benchmark-best-slowdown.pdf")
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
    legend_title="Integer cache",
    output_name="benchmark-integer-cache-effect.pdf",
    legend_fontsize=16,
    legend_title_fontsize=17,
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
    legend_title="Direct linking",
    output_name="benchmark-direct-linking-effect.pdf",
    display_names={
        "false": "disabled",
        "true": "enabled",
    },
    loc="upper left",
    legend_fontsize=15,
    legend_title_fontsize=16,
)

# ------------------------------------------------------------
# Graph 4: best slowdown vs JVM Clojure
#
#
# Intended benchmark configuration:
#   integer cache: any range
#   LLVM optimizations: any level
#   direct linking: any setting
# ------------------------------------------------------------

all_tc = pd.concat(
    [cache_df, llvm_df, direct_df],
    ignore_index=True,
)

plot_best_slowdown(all_tc)

print(f"Wrote figures to {OUT_DIR.resolve()}")
