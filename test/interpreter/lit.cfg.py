import lit.formats

config.name = "llforth"
config.test_format = lit.formats.ShTest()
config.suffixes = ['.fs']
config.substitutions.append(('%{run}', '/bin/cat %s | llforth'))
