# helper to create sample vm binaries for testing
import binascii

def write_hex_to_bin(hexstr: str, outpath: str):
    b = binascii.unhexlify(hexstr)
    with open(outpath, "wb") as f:
        f.write(b)
    print("Wrote", outpath)

# sample1: 3 instrs: invoke main(=1) argc=0; iconst 255; ret
hex1 = "4B41545303000000" + "030100000000" + "01FF000000" + "04"
# sample2: main/addFive example (6 instructions)
hex2 = "4B41545306000000" \
    + "030100000000" \
    + "010A000000" \
    + "030400000001" \
    + "04" \
    + "0105000000" \
    + "02" \
    + "04"

write_hex_to_bin(hex1, "sample1.vm")
write_hex_to_bin(hex2, "sample2.vm")
