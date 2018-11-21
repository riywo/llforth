import lit.formats

config.name = "llforthc"
config.test_format = lit.formats.ShTest()
config.suffixes = ['.forth']

liblib = lit_config.params.get('lib')
config.substitutions.append(('%{lli}', 'lli -load {}'.format(liblib)))
