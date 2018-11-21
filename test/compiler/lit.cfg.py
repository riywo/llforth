import lit.formats

config.name = "llforthc"
config.test_format = lit.formats.ShTest()
config.suffixes = ['.fs']

liblib = lit_config.params.get('lib')
config.substitutions.append(('%{compile}', 'llforthc %s | llc -filetype=obj -o %t.o && clang++ %t.o {} -o'.format(liblib)))
