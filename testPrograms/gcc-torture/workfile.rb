#!/usr/bin/ruby

target = nil
if(ARGV.size > 0 && ARGV[0].end_with?('.c'))
	target = ARGV[0]
	ARGV.delete_at(0)
end

require File.expand_path(ENV['MOSYNCDIR']+'/rules/mosync_exe.rb')
require 'fileutils'
require 'settings.rb'
require 'skipped.rb'

if(target)
	puts "Target: #{target}"
	SETTINGS[:strict_prerequisites] = true
end

BASE = SETTINGS[:base_path]

SP = Struct.new('SourcePath', :base, :path, :mode)

def dg(name)
	return SP.new(name + '/', BASE + name, :dejaGnu)
end

def dgSub(name)
	#p Dir.methods.sort
	#p File.methods.sort
	array = []
	#puts "Scanning #{BASE + name}..."
	Dir.foreach(BASE + name).each do |file|
		path = BASE + name + '/' + file
		if(File.directory?(path) && !file.start_with?('.'))
			array << path
		end
	end
	return array.sort.collect do |dir|
		sp(dir.slice(BASE.length..-1)+'/', dir, :dejaGnu)
	end
end

def sp(base, path, mode = :run)
	return SP.new(base, path, mode)
end

# allowed modes: run, compile, dejaGnu (parse the source file to find compile or run).
SETTINGS[:source_paths] =
	#dgSub('gcc.dg')+
	#dgSub('g++.dg')+
	dgSub('g++.old-deja')+
[
	#dg('c-c++-common/dfp'),	# decimal floating point is not supported.
	dg('c-c++-common/torture'),
	dg('c-c++-common'),
	sp('compile/', BASE + 'gcc.c-torture/compile', :compile),
	sp('unsorted/', BASE + 'gcc.c-torture/unsorted', :compile),
	sp('ieee/', BASE + 'gcc.c-torture/execute/ieee'),
	sp('compat/', BASE + 'gcc.c-torture/compat'),
	sp('', BASE + 'gcc.c-torture/execute'),
]


NEEDS_HEAP = [
'20000914-1.c',
'20020406-1.c',
'20000914-1.c',
'20001203-2.c',
'20020406-1.c',
'20051113-1.c',
'20071018-1.c',
'20071120-1.c',
'920810-1.c',
'941014-2.c',
'960521-1.c',
'990628-1.c',
'comp-goto-1.c',
'ipa-sra-2.c',
'pr15262-1.c',
'pr36765.c',
'pr41395-1.c',
'pr41395-2.c',
'pr41463.c',
'pr42614.c',
'pr43008.c',
'printf-1.c',
'printf-chk-1.c',
'va-arg-21.c',
'vprintf-1.c',
'vprintf-chk-1.c',
'arraynew.C',
'vbase1.C',
]

NEWLIB_NEEDS_HEAP = [
'920501-8.c',
'930513-1.c',
'fprintf-1.c',
'fprintf-chk-1.c',
'struct-ret-1.c',
'vfprintf-1.c',
'vfprintf-chk-1.c',
]

if(USE_NEWLIB)
	NEWLIB_NEEDS_HEAP.each do |n|
		NEEDS_HEAP << n
	end
end

class TTWork < PipeExeWork
	def initialize(f, name)
		super()
		@sourcepath = "#{f.sourcePath.path}/#{name}"
		@sourcefile = f
		@BUILDDIR_PREFIX = String.new(f.sourcePath.base)
		@EXTRA_INCLUDES = ['.'] if(!USE_NEWLIB)
		@EXTRA_SOURCEFILES = [
			@sourcepath,
			#'helpers/helpers.c',
		]
		#@EXTRA_SOURCEFILES << 'helpers/override_heap.c' unless(NEEDS_HEAP.include?(name))
		@EXTRA_OBJECTS = [
			FileTask.new(self, 'build/helpers.o'),
		]
		unless(NEEDS_HEAP.include?(name) || name.end_with?('.C'))
			@EXTRA_OBJECTS << FileTask.new(self, 'build/override_heap.o')
		end
		@SPECIFIC_CFLAGS = {
			# longlong to float conversion is not yet supported.
			'conversion.c' => ' -U __GNUC__',

			'fprintf-chk-1.c' => ' -ffreestanding',
			'vfprintf-chk-1.c' => ' -ffreestanding',
			'pr42833.c' => ' -D__INT_LEAST8_TYPE__=char -D__UINT_LEAST32_TYPE__=unsigned',
			'pr22493-1.c' => ' -fwrapv',
			'pr23047.c' => ' -fwrapv',
			'fp-cmp-1.c' => ' -DSIGNAL_SUPPRESS',
			'fp-cmp-2.c' => ' -DSIGNAL_SUPPRESS',
			'fp-cmp-3.c' => ' -DSIGNAL_SUPPRESS',
			'rbug.c' => ' -D__SPU__',
			'pr47141.c' => ' -D__UINTPTR_TYPE__=unsigned',
			#'raw-string-1.c' => ' -std=gnu99 -trigraphs',
			#'raw-string-10.c' => ' -std=gnu99 -trigraphs',
		}
		@EXTRA_EMUFLAGS = ' -noscreen -allowdivzero'
		@NAME = name
	end
	def define_cflags
		#puts 'define_cflags'
		include_dirs = @EXTRA_INCLUDES
		include_flags = include_dirs.collect {|dir| " -I \""+File.expand_path_fix(dir)+'"'}.join
		flags = ' -g -w'
		flags << ' -O2 -fomit-frame-pointer' if(CONFIG == "")
		flags << ' -ffloat-store -fno-inline' if(@sourcefile.sourcePath.base == 'ieee/')
		flags << include_flags
		@CFLAGS = flags + @EXTRA_CFLAGS
		@CPPFLAGS = flags + @EXTRA_CPPFLAGS

		@TARGET_PATH = @BUILDDIR + @NAME.ext('.moo')
	end
	def builddir; @BUILDDIR; end
	def compile
		setup if(!@CFLAGS)
		FileUtils.mkdir_p(@BUILDDIR)
		makeGccTask(FileTask.new(self, @sourcepath), '.o').invoke
	end
	def invoke
		if(@sourcefile.sourcePath.mode == :dejaGnu)
			@mode = :run	# default
			parseMode
			#puts "Mode #{@mode} for #{@NAME}"
			if(@mode == :run || @mode == :link)
				super
			elsif(@mode == :compile)
				compile
			elsif(@mode == :skip)
				return
			else
				raise "Unknown mode in #{@sourcepath}: #{@mode.inspect}"
#				puts "Unknown mode: #{@mode.inspect}"
#				@mode = :compile
#				compile
			end
		elsif(@sourcefile.sourcePath.mode == :compileOnly)
			compile
		else
			super
		end
	end
	# scans a C file for { dg-do [compile|run] }
	def parseMode
		open(@sourcepath) do |file|
			file.each do |line|
				lineStrip = line.strip
				SKIP_LINES.each do |sl|
					if(lineStrip == sl)
						@mode = :skip
						return
					end
				end

				dgdo = '{ dg-do '
				ts = '{ target '
				xfails = '{ xfail {'
				i = line.index(dgdo)
				if(i)
					ti = line.index(ts)
					if(ti)
						# no tests are targeted at mapip2.
						@mode = :skip
						return
					end
					xi = line.index(xfails)
					if(xi)
						e = xi-1
					else
						e = line.index(' }', i)
					end
					raise "Bad dg-do line!" if(!e)
					@mode = line.slice(i+dgdo.length, e-(i+dgdo.length)).strip.to_sym
					@mode = :compile if(@mode == :assemble)
					next
				end

				dgop = '{ dg-options "'
				i = line.index(dgop)
				if(i)
					i += dgop.length
					e = line.index('"', i)
					raise "Bad dg-options line!" if(!e)
					options = ' ' + line.slice(i, e-i)
					ti = line.index(ts, e)
					#raise "Bad dg-options line in #{@sourcepath}: #{line}" if(!ti)
					if(ti)
						ti += ts.length
						te = line.index(' }', ti)
						raise "Bad dg-options line!" if(!te)
						target = line.slice(ti, te-ti)
						#puts "Found options \"#{options}\" for target #{target}"
						if(target == 'c')
							@EXTRA_CFLAGS = options
						end
						if(target == 'c++')
							@EXTRA_CPPFLAGS = options
						end
					else
						@EXTRA_CFLAGS = options
						@EXTRA_CPPFLAGS = options
					end
					next
				end

				if(line.index('{ dg-error ') || line.index('{ dg-bogus '))
					@mode = :skip
					return
				end
			end
		end
		return nil
	end
	def mode
		return @mode if(@mode)
		m = @sourcefile.sourcePath.mode
		m = :run if(!m)
		return m
	end
end

SourceFile = Struct.new('SourceFile', :sourcePath, :filename)

files = []
SETTINGS[:source_paths].each do |sp|
	pattern = sp.path + '/*.[cC]'
	pattern.gsub!("\\", '/')
	#puts pattern
	Dir.glob(pattern).sort.collect do |fn|
		files << SourceFile.new(sp, fn)
	end
end
puts "#{files.count} files to test:"

builddir = nil
oldBase = nil
targetFound = false

def matchRegexp(tName)
	SKIPPED_REGEXP.each do |r|
		return true if(r.match(tName))
	end
	return false
end

files.each do |f|
	sp = f.sourcePath
	filename = f.filename
	bn = File.basename(filename)
	#p sp, bn
	tName = sp.base + bn
	if(SKIPPED.include?(tName) || matchRegexp(tName))
		#puts "Skipped #{bn}"
		next
	end
	if(target)
		next if(target != (tName))
		targetFound = true
	end
	#puts bn

	builddir = nil if(sp.base != oldBase)

	if(!builddir)
		work = TTWork.new(f, bn)
		work.invoke
		builddir = work.builddir
		oldBase = sp.base
	end

	next if(!builddir)

	ofn = builddir + bn.ext('.o')
	suffix = ''
	pfn = ofn.ext('.moo' + suffix)
	winFile = ofn.ext('.win' + suffix)
	failFile = ofn.ext('.fail' + suffix)
	logFile = ofn.ext('.log' + suffix)
	sldFile = ofn.ext('.sld' + suffix)
	force_rebuild = SETTINGS[:rebuild_failed] && File.exists?(failFile)

	if(SETTINGS[:strict_prerequisites])
		if(!work)
			work = TTWork.new(f, bn)
		end
		work.invoke
	end

	if(force_rebuild)
		FileUtils.rm_f(ofn)
		FileUtils.rm_f(pfn)
		FileUtils.rm_f(winFile)
	end

	winTask = FileTask.new(work, winFile)
	#winTask.prerequisites << FileTask.new(work, ofn)
	if(!winTask.needed?(false))
		#puts "#{bn} won"
		next
	end

	if(!work)
		work = TTWork.new(f, bn)
	end

	begin
		FileUtils.rm_f(winFile)
		FileUtils.touch(failFile)
		if(work.mode == :compile)
			work.compile
		else
			work.invoke
			work.run if(work.mode == :run)
		end
		FileUtils.touch(winFile)
		FileUtils.rm_f(failFile)
	rescue
		FileUtils.touch(failFile)
		FileUtils.rm_f(winFile)
		raise if(SETTINGS[:stop_on_fail])
	end
end

if(target && !targetFound)
	puts "Error: target not found!"
	exit(1)
end
