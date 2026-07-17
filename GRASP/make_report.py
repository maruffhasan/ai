from reportlab.lib.pagesizes import letter
from reportlab.platypus import (SimpleDocTemplate, Paragraph, Spacer, Image,
                                 Table, TableStyle, PageBreak)
from reportlab.lib.styles import getSampleStyleSheet, ParagraphStyle
from reportlab.lib import colors
from reportlab.lib.units import inch
import pandas as pd

STUDENT_ID = "2205100"   # <-- replace with your actual student ID before submitting

df = pd.read_csv("results.csv")

styles = getSampleStyleSheet()
styles.add(ParagraphStyle(name="Justify", parent=styles["Normal"], alignment=4, spaceAfter=8))
title_style = styles["Title"]
h2 = styles["Heading2"]
body = styles["Justify"]

doc = SimpleDocTemplate(f"{STUDENT_ID}.pdf", pagesize=letter,
                         topMargin=0.7*inch, bottomMargin=0.7*inch,
                         leftMargin=0.7*inch, rightMargin=0.7*inch)

story = []

story.append(Paragraph("CSE 318 Assignment 02", title_style))
story.append(Paragraph("Solving the Max-Cut Problem by GRASP", styles["Heading3"]))
story.append(Paragraph(f"Student ID: {STUDENT_ID}", styles["Normal"]))
story.append(Spacer(1, 16))

story.append(Paragraph("1. Algorithm Descriptions", h2))

story.append(Paragraph("<b>Randomized-1:</b> Each vertex is assigned to partition X or Y "
    "independently with probability 1/2. The cut weight is computed and this is repeated "
    "for 30 independent trials; the reported value is the average cut weight, giving an "
    "unbiased baseline for how good a purely random cut is on each graph.", body))

story.append(Paragraph("<b>Greedy-1:</b> The two endpoints of the maximum-weight edge are used "
    "to seed partitions X and Y. The remaining vertices are then scanned once, in index order; "
    "each vertex is placed in whichever partition it currently contributes more edge weight to. "
    "Because vertices are processed in a fixed order rather than by best-contribution-first, this "
    "heuristic is fast.", body))

story.append(Paragraph("<b>Semi-Greedy-1:</b> Uses the same greedy scoring function as above, "
    "but instead of always taking the single best vertex, a Restricted Candidate List (RCL) is "
    "built from all unassigned vertices whose greedy value g(v) = max(&sigma;<sub>X</sub>(v), "
    "&sigma;<sub>Y</sub>(v)) is at least &mu; = w<sub>min</sub> + &alpha;&middot;(w<sub>max</sub> "
    "&minus; w<sub>min</sub>). A vertex is chosen uniformly at random from the RCL and assigned "
    "to its better side. We used &alpha; = 0.5 for the single reported Semi-Greedy-1 value. This "
    "randomization is what allows GRASP to explore many different starting solutions.", body))

story.append(Paragraph("<b>Local Search:</b> Starting from a given partition, for every vertex v "
    "we maintain &delta;(v) = (weight to same-side neighbours) &minus; (weight to opposite-side "
    "neighbours). If the best &delta;(v) is positive, that vertex is flipped to the other side "
    "(a steepest-ascent / best-improvement move), and the &delta; values of its neighbours are "
    "updated in O(deg(v)) time. This repeats until no vertex has a positive &delta;, i.e. a local "
    "optimum is reached. In the CSV, 'Local-1' reports the average local-optimum value over 30 "
    "independent random restarts.", body))

story.append(Paragraph("<b>GRASP:</b> Combines the two ideas above. Each of the GRASP-1 iterations "
    "(60 were used) builds a new randomized solution using Semi-Greedy construction (&alpha; = 0.5) "
    "and immediately improves it with Local Search. The best cut found across all iterations is "
    "kept as the final answer. Because the construction phase is randomized, different iterations "
    "explore different regions of the search space, and the local search phase guarantees each of "
    "those regions is exploited to a local optimum before being compared.", body))

story.append(PageBreak())
story.append(Paragraph("2. Comparison of Algorithms", h2))
story.append(Paragraph(
    "Across all 54 graphs, the best-to-worst order is almost always the same: "
    "<b>GRASP &gt; Local Search &gt; Semi-Greedy &gt; Greedy &gt; Randomized</b>. "
    "GRASP is better than Local Search alone because it tries 60 different starting points and keeps the best one. "
    "Local Search is much better than building a solution once and stopping. "
    "Semi-Greedy and Greedy work poorly when they build solutions without fixing early mistakes. "
    "Randomized just picks sides by coin flip, so it is the weakest. "
    "On the 24 graphs with a known answer, GRASP got the exact right answer on G48 and G49, "
    "and was off by only 1.8% on G50. On average across all 24 graphs, GRASP was about 10.8% away from the best known answer.", body))


story.append(Paragraph("3. Plots", h2))
story.append(Paragraph(
    "The following grouped bar charts show, for each graph, the value obtained by each of the "
    "five algorithms (Randomized average, Greedy, Semi-Greedy, Local Search average, GRASP best).", body))

group_files = [f"plots/group_{i}.png" for i in range(1, 7)]
for gf in group_files:
    story.append(Image(gf, width=6.6*inch, height=3.0*inch))
    story.append(Spacer(1, 10))

story.append(PageBreak())
story.append(Paragraph("GRASP-1 Gap from Known Best / Upper Bound (24 graphs)", h2))
story.append(Paragraph(
    "For the graphs with a published reference value, the chart below shows the percentage gap "
    "between GRASP-1's best value and that reference. The gap is largest (~22-26%) on the sparser, "
    "harder G11-G13 / G32-G34 families, and smallest (0-2%) on the dense, uniform-weight G48-G50 "
    "family, where GRASP reaches the optimum or near-optimum.", body))
story.append(Image("plots/gap_known_best.png", width=6.6*inch, height=3.0*inch))

story.append(PageBreak())
story.append(Paragraph("4. Observations", h2))
story.append(Paragraph(
    "&bull; GRASP consistently outperforms all other methods on every single graph, confirming "
    "the value of combining randomized construction with local search over many iterations.<br/>"
    "&bull; A single Local Search restart already captures most of the achievable improvement over "
    "a random start; GRASP's remaining gain comes from exploring multiple restarts and keeping the best.", body))

story.append(Spacer(1, 20))
story.append(Paragraph("5. Full Results Table", h2))

short_headers = ["Name", "n", "m", "Random\navg", "Greedy", "Semi-\nGreedy",
                 "Local\niters", "Local\navg", "GRASP\niters", "GRASP\nbest", "Known\nbest"]
table_data = [short_headers]
for _, row in df.iterrows():
    vals = []
    for x in row:
        if pd.isna(x):
            vals.append("-")
        elif isinstance(x, float):
            vals.append(f"{x:.1f}")
        else:
            vals.append(str(x))
    table_data.append(vals)

col_widths = [0.45*inch, 0.45*inch, 0.55*inch, 0.62*inch, 0.5*inch, 0.62*inch,
              0.45*inch, 0.62*inch, 0.5*inch, 0.55*inch, 0.55*inch]
t = Table(table_data, repeatRows=1, colWidths=col_widths)
t.setStyle(TableStyle([
    ("BACKGROUND", (0,0), (-1,0), colors.HexColor("#333333")),
    ("TEXTCOLOR", (0,0), (-1,0), colors.white),
    ("FONTSIZE", (0,0), (-1,-1), 6.5),
    ("ALIGN", (1,0), (-1,-1), "CENTER"),
    ("VALIGN", (0,0), (-1,-1), "MIDDLE"),
    ("GRID", (0,0), (-1,-1), 0.25, colors.grey),
    ("ROWBACKGROUNDS", (0,1), (-1,-1), [colors.white, colors.HexColor("#f2f2f2")]),
]))
story.append(t)

doc.build(story)
print("done")