from dataclasses import dataclass

@dataclass
class Instruction:
    op: str
    segment: str | None = None
    arg: int | None = None
    label: str | None = None
    func: str | None = None
