import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
import os

df = pd.read_csv("resultx.csv")
os.makedirs("plots", exist_ok=True)

cols = {
    "Randomized-1 (avg)": "Randomized",
    "Greedy-1": "Greedy",
    "Semi-greedy-1 (alpha=0.5)": "Semi-Greedy",
    "Local-1 average value": "Local Search",
    "GRASP-1 best value": "GRASP",
}

groups = [df.iloc[i:i+9] for i in range(0, len(df), 9)]

for gi, grp in enumerate(groups, start=1):
    names = grp["Name"].tolist()
    x = np.arange(len(names))
    width = 0.15
    fig, ax = plt.subplots(figsize=(11, 5))
    for j, (col, label) in enumerate(cols.items()):
        ax.bar(x + (j - 2) * width, grp[col].values, width, label=label)
    ax.set_xticks(x)
    ax.set_xticklabels(names, rotation=0)
    ax.set_ylabel("Cut weight")
    ax.set_title(f"MAX-CUT algorithm comparison ({names[0]}–{names[-1]})")
    ax.legend()
    ax.grid(axis="y", linestyle="--", alpha=0.4)
    fig.tight_layout()
    fig.savefig(f"plots/group_{gi}.png", dpi=150)
    plt.close(fig)

# Gap-to-known-best plot for the 24 graphs with known values
known = df.dropna(subset=["Known best solution or upper bound"]).copy()
known["gap_pct"] = 100 * (known["Known best solution or upper bound"] - known["GRASP-1 best value"]) / known["Known best solution or upper bound"]
fig, ax = plt.subplots(figsize=(11, 5))
ax.bar(known["Name"], known["gap_pct"], color="#d9534f")
ax.set_ylabel("Gap from known best (%)")
ax.set_title("GRASP-1 percentage gap from known best / upper bound")
ax.grid(axis="y", linestyle="--", alpha=0.4)
plt.xticks(rotation=45)
fig.tight_layout()
fig.savefig("plots/gap_known_best.png", dpi=150)
plt.close(fig)

print("Groups:", len(groups))
print(known[["Name","GRASP-1 best value","Known best solution or upper bound","gap_pct"]].to_string(index=False))
print("Average gap %:", known["gap_pct"].mean())
