import re
import argparse

# Set up argument parsing
parser = argparse.ArgumentParser(description='Parse precision times from a file.')
parser.add_argument('--input_file', nargs='?', default='/home/popina/Programs/C++/disk/gattaca_ant_mania_coding_challenge/code/cpp/out.txt',
                    help='Input file to parse (default: out.txt)')
parser.add_argument('--output_file', default='output.csv',
                    help='Output file to write results (default: output.csv)')
args = parser.parse_args()
print(args.input_file, args.output_file)

# Read the file
with open(args.input_file, 'r') as f:
    content = f.read()

# Extract precision times
pattern = r'Precision Time: (\d+) ns \(([\d.]+) ms\)'
matches = re.findall(pattern, content)

# Create CSV content
csv_header = 'Nanoseconds,Milliseconds\n'
csv_rows = '\n'.join([f'{ns},{ms}' for ns, ms in matches])

# Append to file
with open(args.output_file, 'w') as f:
    f.write(csv_header + csv_rows)

print(f"Extracted {len(matches)} precision time entries and written to {args.output_file}")