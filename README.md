# JSON2RelCSV — Assignment 4

## Goal
Convert any valid JSON file (up to 30 MiB) into relational `.csv` files using Flex, Bison, and C.

## Build Instructions
Make sure `flex` and `bison` are installed.

```bash
sudo apt update
sudo apt install bison flex build-essential
```

```bash
make
```

This creates the binary: `./json2relcsv`

## Usage
```bash
./json2relcsv < input.json
```

Optional flags:
- `--print-ast` → prints the parsed Abstract Syntax Tree

- `--out-dir DIR` → sets output directory (default is current folder)

## Files
- `scanner.l` → tokenizes JSON
- `parser.y` → parses JSON and builds AST
- `ast.*` → defines and prints the AST
- `schema.*` → relational schema creation and CSV output
- `walker.*` → AST-to-table mapping logic
- `main.c` → driver

## Test Cases
### Test 1 — Flat Object

Input:
```json
{ "id": 1, "name": "Ali", "age": 19 }
```

Output root.csv:
```bash
id,name,age
1,"Ali",19
```

### Test 2 — Array of Scalars

Input:
```json
{ "movie": "Inception", "genres": ["Action", "Sci-Fi", "Thriller"] }
```

Output root.csv:
```bash
id,movie
1,"Inception"
```

Output root_genres.csv:
```bash
id,index,value
1,0,"Action"
1,1,"Sci-Fi"
1,2,"Thriller"
```