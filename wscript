#! /usr/bin/env python
# encoding: utf-8

def options(opt):
	opt.load('compiler_cxx')

def configure(ctx):
	ctx.check_waf_version(mini='1.7.9')
	ctx.load('compiler_cxx')
	ctx.env.CXXFLAGS = ['/nologo', '/W4', '/EHsc']
	ctx.env.append_value('INCLUDES', ['.', 'C:\\boost_1_53_0', 'C:\\cryptopp561'])
	ctx.env.append_value('LIBPATH', ['C:\\boost_1_53_0\\stage\\lib32'])
	ctx.env.append_value('DEFINES', ['UNICODE', '_UNICODE'])
	ctx.env.append_value('STLIB', ['advapi32', 'user32', 'kernel32', 'userenv', 'wintrust', 'ole32', 'shlwapi', 'version', 'sfc', 'cryptlib'])
	base_env = ctx.env.derive()

	ctx.setenv('debug', base_env)
	ctx.env.append_value('CXXFLAGS', ['/Od', '/MTd'])
	ctx.env.append_value('STLIBPATH', ['C:\\cryptopp561\\Debug'])

	ctx.setenv('release', base_env)
	ctx.env.append_value('STLIBPATH', ['C:\\cryptopp561\\Release'])
	ctx.env.append_value('DEFINES', ['NDEBUG'])
	ctx.env.append_value('CXXFLAGS', ['/O2', '/GL', '/MT'])
	ctx.env.append_value('LINKFLAGS', ['/LTCG'])
	ctx.env.append_value('ARFLAGS', ['/LTCG'])


def build(ctx):
	if not ctx.variant:
		ctx.fatal('Please select a debug or release build.')

	libSources = ctx.path.ant_glob('pevLib/*.cpp');
	
	ctx.stlib(
		source = libSources,
		target = 'pevLib',
		)
	ctx.program(
		source = ['pevFind/main.cpp'],
		target = 'pevFind',
		use = ['pevLib'],
		includes = ['pevLib']
	)

from waflib.Build import BuildContext
class debug(BuildContext):
	cmd = 'debug'
	variant = 'debug'

class release(BuildContext):
	cmd = 'release'
	variant = 'release'


