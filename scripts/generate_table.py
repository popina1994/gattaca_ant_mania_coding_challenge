import pandas as pd
import matplotlib.pyplot as plt
from matplotlib.backends.backend_pdf import PdfPages
import argparse

def parse_arguments():
    parser = argparse.ArgumentParser(description='Generate optimization report from CSV data.')
    parser.add_argument('--input_csv', type=str, help='Path to the input CSV file (statistics.csv)')
    parser.add_argument('--output_pdf', type=str, help='Path to the output PDF file (optimization_report.pdf)')
    return parser.parse_args()

def create_pdf_report(input_csv, output_pdf):
    df = pd.read_csv(input_csv)

    # Format integer and float columns
    int_cols = ['Seed', 'Ants', 'Optimization Level']
    float_cols = ['NS Mean', 'NS Std', 'MS Mean', 'MS Std']

    # Convert integers
    df[int_cols] = df[int_cols].astype(int)

    # Convert floats to scientific notation strings
    for col in float_cols:
        df[col] = df[col].apply(lambda x: f"{x:.2e}")

    with PdfPages(output_pdf) as pdf:
        create_table(df, pdf)
        create_plots(df, float_cols[-2:], pdf)

def create_table(df, pdf):
    fig, ax = plt.subplots(figsize=(12, 5))
    ax.axis('tight')
    ax.axis('off')
    table_data = df.values
    col_labels = df.columns
    table = ax.table(cellText=table_data, colLabels=col_labels, loc='center')
    table.auto_set_font_size(False)
    table.set_fontsize(10)
    table.scale(1, 1.5)
    pdf.savefig(fig, bbox_inches='tight')
    plt.close()

def create_plots(df, float_cols, pdf):
    # Convert float columns back to numeric for plotting
    df_plot = df.copy()
    for col in float_cols:
        df_plot[col] = pd.to_numeric(df_plot[col], errors='coerce')

    for y_column in float_cols:
        fig, ax = plt.subplots(figsize=(10, 5))

        # Loop over unique Seed and Ants combinations
        for seed in df_plot['Seed'].unique():
            for ants in df_plot['Ants'].unique():
                subset = df_plot[(df_plot['Seed'] == seed) & (df_plot['Ants'] == ants)]
                if not subset.empty:
                    ax.plot(subset['Optimization Level'], subset[y_column],
                            marker='o', label=f'Seed {seed}, Ants {ants}')

        ax.set_title(f'{y_column} vs Optimization Level')
        ax.set_xlabel('Optimization Level')
        ax.set_ylabel(y_column)
        ax.grid(True)

        # Move legend below the plot
        ax.legend(loc='upper center', bbox_to_anchor=(0.5, -0.15),
                  fancybox=True, shadow=False, ncol=3)

        # Use scientific notation for y-axis
        ax.ticklabel_format(axis='y', style='sci', scilimits=(0,0))

        pdf.savefig(fig, bbox_inches='tight')
        plt.close()

if __name__ == "__main__":
    args = parse_arguments()
    create_pdf_report(args.input_csv, args.output_pdf)
    print(f"PDF report created: {args.output_pdf}")