import pathlib;

def count_lines_and_semicolons(file: pathlib.Path) -> tuple[int, int]:
    line_count = 0
    semicolon_count = 0

    try:
        with file.open('r', encoding='utf-8') as f:
            for line in f:
                line_count += 1
                semicolon_count += line.count(';')
    except Exception as e:
        print(f"Skipping {file}: {e}")
    
    return line_count, semicolon_count

def main():
    path = pathlib.Path('src')
    total_lines = 0
    total_semicolons = 0

    for file in path.rglob('*'):
        if file.is_file():
            loc, semicolons = count_lines_and_semicolons(file)
            total_lines += loc
            total_semicolons += semicolons

    print(f"LOC: {total_lines}")
    print(f"SEMICOLONS: {total_semicolons}")

if __name__ == '__main__':
    main()
