import argparse
import util
import sys
import os

os.chdir(os.path.dirname(__file__))


parser = argparse.ArgumentParser(description='Resource tool used to build zelda3_assets.dat', allow_abbrev=False)
parser.add_argument('-r', '--rom', nargs='?', metavar='ROM')
parser.add_argument('--extract-from-rom', '-e', action='store_true', help='Extract/overwrite things from the ROM')

optional = parser.add_argument_group('Language settings')
optional.add_argument('--extract-dialogue', action='store_true', help = 'Extract dialogue from a translated ROM')
optional.add_argument('--languages', action='store', metavar='L1,L2', help = 'Comma separated list of additional languages to build (de,fr,fr-c,en,es,pl,pt,redux,nl,sv).')

optional = parser.add_argument_group('Debug things')
optional.add_argument('--no-build', action='store_true', help="Don't actually build zelda3_assets.dat")
optional.add_argument('--print-strings', action='store_true', help="Print all dialogue strings")
optional.add_argument('--print-assets-header', action='store_true')

optional = parser.add_argument_group('Image handling')
optional.add_argument('--sprites-from-png', action='store_true', help="When compiling, load sprites from png instead of from ROM")

args = parser.parse_args()

if args.extract_dialogue:
  ROM = util.load_rom(args.rom, True)
  import extract_resources, sprite_sheets
  sprite_sheets.decode_font()
  extract_resources.print_dialogue()
  sys.exit(0)

ROM = util.load_rom(args.rom)

want_compile = True

if args.extract_from_rom:
  import extract_resources
  extract_resources.main()

if args.print_strings:
  import text_compression
  text_compression.print_strings(ROM)
  want_compile = False

if want_compile and not args.no_build:
  import compile_resources
  compile_resources.main(args)



