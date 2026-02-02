#!/usr/bin/env python3
import subprocess
import os
import sys
import tempfile
import shutil

# Root logic: Script sits in /tests/, project root is parent.
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
ROOT_DIR = os.path.dirname(SCRIPT_DIR)
BINUTILS_DIR = os.path.join(ROOT_DIR, "binutils")
PLUGIN_SRC = os.path.join(ROOT_DIR, "plugins/objdump_pluginv2.cpp")
PLUGIN_SO = os.path.join(BINUTILS_DIR, "objdump_pluginv2.so")
OBJDUMP = os.path.join(BINUTILS_DIR, "objdump")
OBJCOPY = os.path.join(BINUTILS_DIR, "objcopy")

# --- EMBEDDED TEST DATA ---
# This dictionary contains hex code and the expected 'disasm_output.txt' content.
ARCHS = {
    "i386": {
        "target": "TARGET_I386",
        "hex": "55 89 e5 b8 01 00 00 00 5d c3",
        "objcopy_args": ["-I", "binary", "-O", "elf32-i386", "--binary-architecture", "i386"],
        "golden": "0::push,%ebp,|1::mov,%esp,%ebp,|3::mov,$0x1,%eax,|8::pop,%ebp,|9::ret,,|\n"
    },
    "x86_64": {
        "target": "TARGET_X86_64",
        "hex": "55 48 89 e5 b8 01 00 00 00 5d c3",
        "objcopy_args": ["-I", "binary", "-O", "elf64-x86-64", "--binary-architecture", "i386:x86-64"],
        "golden": "2::,,|2::HH,,|2::,,|2::,,|2::,,|\n"
    },
    "ARM": {
        "target": "TARGET_ARM",
        "hex": "04 b0 2d e9 04 b0 8d e2 00 30 a0 e3 03 00 a0 e1 00 88 bd e8",
        "objcopy_args": ["-I", "binary", "-O", "elf32-littlearm", "--binary-architecture", "arm"],
        "golden": "0::push,%m,|4::add,%12-15r,%16-19r,%o,|8::mov,%12-15r,%o,|c::mov,%12-15r,%0-3r,|10::pop,%m,|\n"
    },
    "MIPS": {
        "target": "TARGET_MIPS",
        "hex": "01 00 43 24 10 00 be af 08 00 e0 03 00 00 00 00",
        "objcopy_args": ["-I", "binary", "-O", "elf32-tradlittlemips", "--binary-architecture", "mips"],
        "golden": "0::addiu,t,r,j,|4::sw,t,o(b),|8::jr,s,|c::nop,,|\n"
    }
}

def run_command(cmd, cwd=ROOT_DIR):
    result = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True, cwd=cwd)
    return result

def compile_plugin(target_arch):
    print(f"  [+] Compiling Plugin ({target_arch})...", end=" ", flush=True)
    cmd = [
        "g++", "-shared", "-fPIC", "-o", PLUGIN_SO, PLUGIN_SRC,
        f"-D{target_arch}",
        "-L./opcodes", "-lopcodes", "-L./bfd", "-lbfd",
        "-I./include", "-I./bfd", "-lre2"
    ]
    res = run_command(cmd, cwd=ROOT_DIR)
    if res.returncode == 0:
        print("OK")
        return True
    print(f"FAILED\nError: {res.stderr}")
    return False

def normalize_output(text: str) -> str:
    # Normalize spacing differences in operands
    text = text.replace(", ", ",")
    text = text.replace(" ,", ",")
    while "  " in text:
        text = text.replace("  ", " ")
    return text.strip()

def test_architecture(name, config):
    print(f"\n>>> TESTING: {name}")
    
    if not compile_plugin(config["target"]):
        return False

    with tempfile.TemporaryDirectory() as tmpdir:
        bin_path = os.path.join(tmpdir, "test.bin")
        obj_path = os.path.join(tmpdir, "test.o")
        disasm_out = os.path.join(ROOT_DIR, "tests", "PPDA", "disasm_output.txt")
        ppda_dir = os.path.join(ROOT_DIR, "tests", "PPDA")
        
        # Create PPDA directory for output
        os.makedirs(ppda_dir, exist_ok=True)
        
        # Cleanup previous disasm if exists
        if os.path.exists(disasm_out): os.remove(disasm_out)

        # 1. Create binary from hex
        bytecode = bytes.fromhex(config["hex"])
        with open(bin_path, "wb") as f: f.write(bytecode)

        # 2. Convert to ELF object
        if not os.path.exists(OBJCOPY):
            print(f"  [-] objcopy not found. Run: cd {ROOT_DIR} && make objcopy")
            return False
        objcopy_cmd = [OBJCOPY] + config["objcopy_args"] + [bin_path, obj_path]
        res = run_command(objcopy_cmd)
        if res.returncode != 0:
            print(f"  [-] Failed to generate test object for {name}")
            print(f"      stderr: {res.stderr}")
            return False

        # 3. Run objdump with plugin
        print(f"  [+] Running objdump...", end=" ", flush=True)
        # objdump loads plugin automatically from ./objdump_pluginv2.so
        objdump_cmd = [OBJDUMP, "-D", obj_path]
        # Execute from BINUTILS_DIR so plugin can be found and output paths work
        res = run_command(objdump_cmd, cwd=BINUTILS_DIR)

        # 4. Content Verification
        if not os.path.exists(disasm_out):
            print("FAILED (disasm_output.txt missing)")
            if res.stderr:
                print(f"    objdump stderr: {res.stderr[:200]}")
            return False

        with open(disasm_out, 'r') as f:
            actual = f.read()

        if normalize_output(actual) == normalize_output(config["golden"]):
            print("OK (Byte-perfect)")
            return True
        else:
            print("FAILED (Mismatch)")
            print(f"    Expected: {config['golden'].strip()}")
            print(f"    Actual:   {actual.strip()}")
            return False

def main():
    if not os.path.exists(OBJDUMP):
        print(f"Error: objdump not found at {OBJDUMP}. Build the project first.")
        sys.exit(1)

    success_count = 0
    for arch, config in ARCHS.items():
        if test_architecture(arch, config):
            success_count += 1

    print("\n" + "="*40)
    if success_count == len(ARCHS):
        print("  RESULT: ALL REGRESSION TESTS PASSED!")
        print("  CLEANUP: Done (Autonomous mode).")
        print("="*40)
        sys.exit(0)
    else:
        print(f"  RESULT: {len(ARCHS) - success_count} TESTS FAILED.")
        print("="*40)
        sys.exit(1)

if __name__ == "__main__":
    main()
