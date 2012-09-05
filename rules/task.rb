# Copyright (C) 2009 Mobile Sorcery AB
#
# This program is free software; you can redistribute it and/or modify it under
# the terms of the GNU General Public License, version 2, as published by
# the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
# or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
# for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; see the file COPYING.  If not, write to the Free
# Software Foundation, 59 Temple Place - Suite 330, Boston, MA
# 02111-1307, USA.

# This is the base file for the Work system.
# Work is a library designed to help generate new files from existing source files.
# In other words, a build system.
# Work shares some similarities with <em>make</em> and <em>rake</em>,
# especially in its base structure, but have many additions.
# It's designed to be fast; minimizing the number of processes started and
# the number of file system accesses.
#
# Author:: Fredrik Eldh (mailto:fredrik.eldh@mobilesorcery.com)
# Copyright:: Copyright (c) 2009 Mobile Sorcery AB
# License:: GNU GPL, version 2

require "#{File.dirname(__FILE__)}/defaults.rb"
require "#{File.dirname(__FILE__)}/targets.rb"
require "#{File.dirname(__FILE__)}/host.rb"
require "#{File.dirname(__FILE__)}/util.rb"
require 'fileutils'

# This is the base class for the Work system.
class TaskBase
	def initialize
		@prerequisites = []
	end

	# An array of TaskBase objects that must be invoked before this object can be executed.
	attr_accessor(:prerequisites)

	# Writes a readable representation of this TaskBase and its prerequisites to stdout.
	def dump(level)
		@prerequisites.each do |p| p.dump(level+1) end
	end
end

# A Work represents the end result of a build process.
# This is often an executable file or a library.
# A Work is not quite a Task. It has prerequisites and responds to invoke, but that's it.
# Work also cooperates with the Target system.
# Work is abstract; subclasses must define the setup and execute_clean methods.
class Work < TaskBase
	include Defaults
	def initialize
		Targets.setup
		super
	end

	def invoke
		#puts "Work.invoke: #{@NAME.inspect}"

		if(@prerequisites == [] || !@prerequisites) then
			setup
			if(@prerequisites == [])
				error "setup failed"
			end
		end

		# If you invoke a work without setting up any targets,
		# we will check for the "clean" goal here.
		# Let's also check for "run".
		doRun = false
		if(Targets.size == 0)
			Targets.setup
			if(Targets.goals.include?(:clean))
				self.execute_clean
				return
			end
			doRun = true if(Targets.goals.include?(:run))
		end

		@prerequisites.each do |p| p.invoke end

		if(doRun)
			if(self.respond_to?(:run))
				run
			else
				cmd = @TARGET.to_s
				if(defined?(EXTRA_RUNPARAMS))
					cmd << " #{EXTRA_RUNPARAMS}"
				end
				sh cmd
			end
		end
	end

	def invoke_clean
		if(@prerequisites == []) then
			setup
			if(@prerequisites == [])
				error "setup failed"
			end
		end

		self.execute_clean
	end

	# Invoke the workfile of another directory, as if it would have been called from the command line.
	def Work.invoke_subdir(dir, *args)
		Work.invoke_subdir_ex(false, dir, *args)
	end
	def Work.invoke_subdir_ex(reload, dir, *args)
		puts File.expand_path(dir) + " " + args.inspect
		fn = dir + "/workfile.rb"
		if(!File.exists?(fn))
			error "No workfile found in #{dir}"
		end

		if(defined?(Targets))
			Targets.reset(args)
		end

		oldDir = Dir.getwd
		Dir.chdir(dir)
		if(RELOAD || reload)
			args = args.join(' ')
			if(USE_NEWLIB)
				args << ' USE_NEWLIB='
			end
			if(USE_ARM)
				args << ' USE_ARM='
			end
			if(FULLSCREEN == "true")
				args << " FULLSCREEN=\"true\""
			end
			args << ' RELOAD=' if(RELOAD)
			args << " PACK=#{PACK}" if(defined?(PACK))
			args << " MODE=#{MODE}" if(defined?(MODE))
			args << " CONFIG=#{CONFIG}" if(!args.include?('CONFIG'))
			cmd = "workfile.rb #{args}"
			if(HOST == :win32)
				sh "ruby #{cmd}"
			else
				sh "./#{cmd}"
			end
		else
			load(File.expand_path('workfile.rb'), true)
		end
		Dir.chdir(oldDir)
	end

	def Work.invoke_subdirs(dirs, *args)
		Work.invoke_subdirs_ex(false, dirs, *args)
	end
	def Work.invoke_subdirs_ex(reload, dirs, *args)
		dirs.each do |dir| Work.invoke_subdir_ex(reload, dir, *args) end
	end
end

# Represents an ordinary build process, where new files are created in a designated directory.
# Implements setup, but requires subclasses to define setup2.
# Includes a set of default member variables used by the different subclasses.
# To override the default value of such a variable, it must be set before setup is called.
class BuildWork < Work

	def setup
		#puts "BuildWork.setup: #{@NAME.inspect}"
		set_defaults
		@prerequisites << DirTask.new(self, @BUILDDIR)
		if(@TARGETDIR)
			@prerequisites << DirTask.new(self, @TARGETDIR)
			@prerequisites << DirTask.new(self, @TARGETDIR + '/' + @BUILDDIR)
		end
		@prerequisites += @PREREQUISITES
		setup2
		#dump(0)
		if(@INSTALLDIR)
			@prerequisites << DirTask.new(self, @INSTALLDIR)
			@prerequisites << CopyFileTask.new(self, @INSTALLDIR + '/' + File.basename(@TARGET), @TARGET)
		end
	end

	# Removes all files generated by this Work.
	def execute_clean
		#puts "execute_clean in #{self.inspect}"
		verbose_rm_rf(@TARGET)
		verbose_rm_rf(@BUILDDIR)
	end
end

# This is the base class for general-purpose tasks.
# @work is the Work to which this Task is attached.
class Task < TaskBase
	def initialize(work)
		super()
		@work = work
	end

	# Invokes this Task. First invokes all prerequisites, then
	# calls <tt>execute</tt> to perform whatever actions are necessary.
	# <tt>execute</tt> is not implemented in the base classes; one must create subclasses
	# and implement it.
	# Returns true if the Task was executed, false otherwise.
	def invoke
		#puts "invoke: #{self}"
		@prerequisites.each do |p| p.invoke end
		if(self.needed?) then
			puts "Execute: #{self}"
			self.execute
			return true
		end
		return false
	end

	# A Task's timestamp is used for comparison with other Tasks to determine
	# if a target is older than a source and thus needs to be remade.
	# This default implementation returns EARLY.
	def timestamp
		EARLY
	end

	# Returns true if this Task should be executed, false otherwise.
	def needed?(log = true)
		true
	end
end

# A Task representing a file.
# @NAME is the name of the file.
class FileTask < Task
	def initialize(work, name)
		super(work)
		@NAME = name.to_s
		# names may not contain '~', the unix home directory hack, because File.exist?() doesn't parse it.
		if(@NAME.start_with?('~'))
			error "Bad filename: #{@NAME}"
		end
	end

	def to_str
		@NAME
	end

	def to_s
		@NAME
	end

	def execute
		error "Don't know how to build #{@NAME}"
	end

	# Makes sure the execution did not fail silently.
	def invoke
		res = super
		if(res)
			error "Failed to build #{@NAME}" if(needed?(true))
		end
		return res
	end

	# Is this FileTask needed?  Yes if it doesn't exist, or if its time stamp
	# is out of date.
	# Prints the reason the task is needed, if <tt>log</tt>.
	def needed?(log = true)
		if(!File.exist?(@NAME))
			puts 'In '+FileUtils.pwd if(log && PRINT_WORKING_DIRECTORY)
			puts "Because file does not exist:" if(log)
			return true
		end
		return true if out_of_date?(timestamp, log)
		return false
	end

	def self.timestamp(file)
		if File.exist?(file)
			File.mtime(file)
		else
			LATE
		end
	end

	# Time stamp for file task. If the file exists, this is the file's modification time,
	# as reported by the filesystem.
	def timestamp
		self.class.timestamp(@NAME)
	end

	# Are there any prerequisites with a later time than the given time stamp?
	def out_of_date?(stamp, log=true)
		@prerequisites.each do |n|
			if(n.timestamp > stamp)
				puts "Because prerequisite '#{n}'(#{n.class}) is newer:" if(log)
				return true
			end
		end
		return false
	end

	def dump(level)
		puts (" " * level) + @NAME
		super
	end
end

# A Task representing multiple files.
# If any of the files are out-of-date, the Task will be executed.
# The first file is designated primary, and acts as the single file in the parent class, FileTask.
class MultiFileTask < FileTask
	def initialize(work, name, files)
		super(work, name)
		@files = files.collect do |f|
			fn = f.to_s
			# names may not contain '~', the unix home directory hack, because File.exist?() doesn't parse it.
			if(fn.include?('~'))
				error "Bad filename: #{fn}"
			end
			fn
		end
	end

	def invoke
		res = super
		if(res)
			@files.each do |file|
				error "Failed to build #{file}" if(out_of_date?(self.class.timestamp(file), true))
			end
		end
		return res
	end

	def needed?(log = true)
		if(!File.exist?(@NAME))
			puts "Because file does not exist:" if(log)
			return true
		end
		@files.each do |file|
			if(!File.exist?(file))
				puts "Because secondary file '#{file}' does not exist:" if(log)
				return true
			end
		end
		return true if out_of_date?(timestamp, log)
		return false
	end

	# Returns the timestamp of the newest file.
	def timestamp
		time = super
		@files.each do |file|
			t = self.class.timestamp(file)
			time = t if(t > time)
		end
		return time
	end
end

# A Task for creating a directory, recursively.
# For example, if you want to create 'foo/bar', you need not create two DirTasks. One will suffice.
class DirTask < FileTask
	def execute
		p @NAME
		FileUtils.mkdir_p @NAME
	end
	def timestamp
		if File.directory?(@NAME)
			t = EARLY
		else
			t = LATE
		end
		#puts "Timestamp(#{@NAME}): #{t}"
		#p File.directory?(@NAME)
	end
end

# A Task for copying a file.
class CopyFileTask < FileTask
	# name is a String, the destination filename.
	# src is a FileTask, the source file.
	# preq is an Array of Tasks, extra prerequisites.
	def initialize(work, name, src, preq = [])
		super(work, name)
		@src = src
		@prerequisites += [src] + preq
	end
	def execute
		puts "copy #{@src} #{@NAME}"
		FileUtils.copy_file(@src, @NAME, true)
		# Work around a bug in Ruby's utime, which is called by copy_file.
		# Bug appears during Daylight Savings Time, when copying files with dates outside DST.
		mtime = File.mtime(@src)
		if(!mtime.isdst && Time.now.isdst)
			mtime += mtime.utc_offset
			File.utime(mtime, mtime, @NAME)
		end
	end
end

# generate file in memory, then compare it to the one on disk
# to decide if it should be overwritten.
# subclasses must provide member variable @buf.
class MemoryGeneratedFileTask < FileTask
	def initialize(work, name)
		super(work, name)
		@ec = open(@NAME).read if(File.exist?(@NAME))
	end
	def needed?(log = true)
		return true if(super(log))
		if(@buf != @ec)
			puts "Because generated file has changed:" if(log)
			return true
		end
		return false
	end
	def execute
		file = open(@NAME, 'w')
		file.write(@buf)
		file.close
		@ec = @buf
	end
end
