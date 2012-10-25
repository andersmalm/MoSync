#!/usr/bin/ruby

require File.expand_path('../../rules/native_mosync.rb')
require File.expand_path('../../rules/mosync_util.rb')

work = MoSyncExe.new
work.instance_eval do
	@SOURCES = ['.']
	@EXTRA_SOURCEFILES = [
		'../../runtimes/cpp/platforms/sdl/FileImpl.cpp',
		'../../runtimes/cpp/base/FileStream.cpp',
	]
	@PREREQUISITES = [
	]
	@EXTRA_SOURCETASKS = [
	]
	@EXTRA_INCLUDES = [
		'../../intlibs',
		'../../runtimes/cpp/base',
		'../../runtimes/cpp/platforms/sdl',
	]
	@LOCAL_LIBS = ['mosync_log_file']
	if(HOST == :linux)
		@LIBRARIES = ['pthread']
	end

	@NAME = 'insmapRead'

	@INSTALLDIR = mosyncdir + '/bin'
end

work.invoke
