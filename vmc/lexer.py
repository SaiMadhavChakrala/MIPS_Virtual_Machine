def tokenize_line(line: str):
    line = line.split("//")[0].strip()
    if not line:
        return None
    return line.split()

def lex_file(text: str):
    tokens = []
    for line in text.splitlines():
        t = tokenize_line(line)
        if t:
            tokens.append(t)
    return tokens
