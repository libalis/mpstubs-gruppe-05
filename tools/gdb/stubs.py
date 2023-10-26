
import sys
# path to local stubs package
sys.path.insert(1, './tools/gdb/')
# path to checkout of qmp package 'https://pypi.org/project/qemu.qmp/'
sys.path.insert(1, '/proj/i4stubs/tools/python/')
try:
    import stubs
except Exception as e:
    print(f"could not load gdb stubs plugin: {str(e)}", file=sys.stderr)
    pass
