#!/usr/bin/ruby

require File.expand_path('../../rules/native_mosync.rb')

rcomp = MoSyncExe.new
rcomp.instance_eval do
	@SOURCES = ['src']
	@EXTRA_INCLUDES = ['inc', 'src']
	@IGNORED_FILES = ['linkarra.cpp']
	@EXTRA_CPPFLAGS = ' -D__MSVCDOTNET__ -Wno-shadow -Wno-missing-noreturn -Wno-missing-format-attribute'
	if(HOST == :linux)
		@EXTRA_CPPFLAGS += ' -D__LINUX__'
	end
	@SPECIFIC_CFLAGS = {
		'rcompl.cpp' => ' -Wno-unused-function -Wno-undef',
		'rcomp.cpp' => ' -Wno-unused-function -Wno-undef',
		'numval.cpp' => ' -Wno-float-equal',
	}
	#@LIBRARIES = ['z']
	@NAME = 'rcomp'
	@INSTALLDIR = mosyncdir + '/bin'
	def setup
		set_defaults
		@SPECIFIC_CFLAGS['rcomp.cpp'] << ' -Wno-delete-non-virtual-dtor' if(@GCC_V4_SUB >= 7)
		super
	end
end

rcomp.invoke
