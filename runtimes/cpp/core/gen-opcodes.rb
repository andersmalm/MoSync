#!/usr/bin/ruby

# This program generates numeric values of opcodes in the Mapip2 arch.
# Outputs header files with enums in the following languages:
# CGEN
# C
# custom binutils description

Operand = Struct.new('Operand', :type, :offset)
Opcode = Struct.new('Opcode', :name, :mnemonic, :operands)

def o(n,m,o)
	Opcode.new(n,m,generateOffsetOperands(o))
end

def isRegister(opType)
	case opType
	when :rd, :rs, :frd, :frs
		return true
	else
		return false
	end
end

def generateOffsetOperands(o)
	# first count the registers.
	nreg = 0;
	o.each do |op|
		nreg += 1 if(isRegister(op) || op == :adaddr)
	end
	# then set the offsets
	regOffset = 1
	immOffset = 1 + nreg
	ops = []
	o.each do |op|
		if(isRegister(op))
			offset = regOffset
			regOffset += 1
		else
			offset = immOffset
			immOffset += ((op == :imm8) ? 1 : 4)
		end
		ops << Operand.new(op, offset)
	end
	return ops
end

OPCODES = [
	o(:nop, 'nop', []),	# NOP must be the first one, number 0.
	o(:break, 'break', []),	# debugger breakpoint.
	o(:ldr, 'ld', [:rd, :rs]),
	o(:lddr, 'ld.d', [:rd, :rs]),
	o(:lddi, 'ld.d', [:rd, :fimmd]),
	o(:ldi, 'ld', [:rd, :imm]),
	o(:ldw, 'ld', [:rd, :adaddr]),
	o(:ldb, 'ld.b', [:rd, :adaddr]),
	o(:ldh, 'ld.h', [:rd, :adaddr]),
	o(:ldd, 'ld.d', [:rd, :adaddr]),
	o(:stw, 'ld', [:adaddr, :rs]),
	o(:stb, 'ld.b', [:adaddr, :rs]),
	o(:sth, 'ld.h', [:adaddr, :rs]),
	o(:std, 'ld.d', [:adaddr, :rs]),
	o(:case, 'case', [:rd, :imm, :imm, :imm, :aiaddr]),
	o(:jpi, 'jp', [:aiaddr]),
	o(:callr, 'call', [:rd]),
	o(:calli, 'call', [:aiaddr]),
	o(:syscall, 'syscall', [:imm8]),
	o(:ret, 'ret', []),
	o(:push, 'push', [:rd, :rs]),
	o(:pop, 'pop', [:rd, :rs]),
	o(:not, 'not', [:rd, :rs]),
	o(:neg, 'neg', [:rd, :rs]),
	o(:xh, 'xh', [:rd, :rs]),
	o(:xb, 'xb', [:rd, :rs]),

	# floating-point
	o(:fpush, 'push', [:frd, :frs]),
	o(:fpop, 'pop', [:frd, :frs]),
	o(:fldis, 'ld', [:frd, :fimms]),	# float load immediate single (32 bits)
	o(:fldid, 'ld.d', [:frd, :fimmd]),	# float load immediate double (64 bits)
	o(:fsts, 'ld.s', [:adaddr, :frs]),	# float store single
	o(:fstd, 'ld.d', [:adaddr, :frs]),	# float store double
	o(:flds, 'ld.s', [:frd, :adaddr]),	# float load single
	o(:fldd, 'ld.d', [:frd, :adaddr]),	# float load double
	o(:fldr, 'ld', [:frd, :frs]),	# float load register
	o(:fldrs, 'ld', [:frd, :rs]),	# float load integer register single (32 bits, direct copy without conversion)
	o(:fldrd, 'ld.d', [:frd, :rs]),	# float load integer register double (rs & rs+1)
	o(:fstrs, 'ld', [:rd, :frs]),	# float store integer register single
	o(:fstrd, 'ld.d', [:rd, :frs]),	# float store integer register double
	o(:floats, 'float.s', [:frd, :rs]),	# float convert from 32-bit int
	o(:floatd, 'float.d', [:frd, :rs]),	# float convert from 64-bit int (rs & rs+1)
	o(:floatuns, 'floatun.s', [:frd, :rs]),	# float convert from 32-bit unsigned
	o(:floatund, 'floatun.d', [:frd, :rs]),	# float convert from 64-bit unsigned (rs & rs+1)
	o(:fix_truncs, 'fix_trunc.s', [:rd, :frs]),	# float convert to 32-bit int
	o(:fix_truncd, 'fix_trunc.d', [:rd, :frs]),	# float convert to 64-bit int (rd & rd+1)
	o(:fixun_truncs, 'fixun_trunc.s', [:rd, :frs]),	# float convert to 32-bit unsigned
	o(:fixun_truncd, 'fixun_trunc.d', [:rd, :frs]),	# float convert to 64-bit unsigned (rd & rd+1)
]

FLOAT_ARITH_OPCODES = [
	:fadd,
	:fsub,
	:fmul,
	:fdiv,
	:fsqrt,
	:fsin,
	:fcos,
	:fexp,
	:flog,
	:fpow,
	:fatan2,
]

JC_OPCODES = [
	:eq,
	:ne,
	:ge,
	:gt,
	:le,
	:lt,
]
JCU_OPCODES = [
	:ltu,
	:geu,
	:gtu,
	:leu,
]

def intjc(op)
	OPCODES << o(('jc_' + op.to_s).to_sym, 'jc '+op.to_s, [:rd, :rs, :riaddr])
end

JC_OPCODES.each do |op|
	intjc(op)
	OPCODES << o(('fjc_' + op.to_s).to_sym, 'fjc '+op.to_s, [:frd, :frs, :riaddr])
end

JCU_OPCODES.each do |op|
	intjc(op)
end

ARITH_OPCODES = [
	:div,
	:divu,
	:mul,
	:add,
	:sub,
	:and,
	:or,
	:xor,
]

SHIFT_OPCODES = [
	:sll,
	:sra,
	:srl,
]

ARITH_OPCODES.each do |op|
	OPCODES << o(op, op.to_s, [:rd, :rs])
	i = op.to_s + 'i'
	OPCODES << o(i.to_sym, op.to_s, [:rd, :imm])
end

SHIFT_OPCODES.each do |op|
	OPCODES << o(op, op.to_s, [:rd, :rs])
	i = op.to_s + 'i'
	OPCODES << o(i.to_sym, op.to_s, [:rd, :imm8])
end

FLOAT_ARITH_OPCODES.each do |op|
	OPCODES << o(op, op.to_s, [:frd, :frs])
end

puts "Count: #{OPCODES.size} opcodes"


REGISTERS = [
:zr,:sp,:ra,:fp,
:s0,:s1,:s2,:s3,:s4,:s5,:s6,:s7,
:p0,:p1,:p2,:p3,
:g0,:g1,:g2,:g3,:g4,:g5,:g6,:g7,
:g8,:g9,:g10,:g11,:g12,:g13,
:r0,:r1,
]

FREG_COUNT = 16


mode = ARGV[0]
filename = ARGV[1]

puts "Mode: #{mode}"
puts "Filename: #{filename}"

def nodePart(name, children_or_opcode)
	name = name.gsub('.', '_')
	name = name.gsub(' ', '_')
	if(children_or_opcode.is_a? Hash)
		return "#{name}, ARRAY_SIZE(#{name})"
	else
		return "NULL, OP_#{children_or_opcode.to_s.upcase}"
	end
end

def outputParseNode(file, prefix, op)
	prefix = prefix.gsub('.', '_')
	prefix = prefix.gsub(' ', '_')
	if(op.is_a? Hash)
		op.each do |operand, children|
			outputParseNode(file, prefix + '_' + operand.type.to_s, children)
		end
		file.puts "static const mapip2_parse_node _#{prefix}[] = {"
		op.each do |operand, children_or_opcode|
			c = nodePart("_#{prefix}_#{operand.type}", children_or_opcode)
			file.puts "\t{ #{operand.type.to_s.upcase}, #{c}, #{operand.offset} },"
		end
		file.puts '};'
	end
end

def writeRegisterNames(file)
	file.puts
	file.puts 'const char* const mapip2_register_names[] = {'
	REGISTERS.each do |r|
		file.puts "\t\"#{r}\","
	end
	file.puts '};'
	file.puts
	file.puts 'const char* const mapip2_float_register_names[] = {'
	(0..FREG_COUNT-1).each do |i|
		file.puts "\t\"f#{i}\","
	end
	file.puts '};'
end

def writeCCoreInstructions(file, &reg)
	i = 0
	file.puts '#ifndef GEN_OPCODES_H'
	file.puts '#define GEN_OPCODES_H'
	file.puts
	OPCODES.each do |op|
		file.puts "#define OP_#{op.name.to_s.upcase} #{sprintf('0x%x', i)}"
		i += 1
	end
	file.puts
	file.puts "#define OPCODE_COUNT #{OPCODES.size}"
	file.puts
	file.puts "#define INSTRUCTIONS(m)\\"
	OPCODES.each do |op|
		file.puts "\tm(#{op.name.to_s.upcase})\\"
	end
	file.puts
	reg.call
	file.puts
	file.puts '#endif	//GEN_OPCODES_H'
end

if(mode == 'cgen')
	open(filename, 'w') do |file|
		i = 0
		file.puts '(define-normal-insn-enum opcodes "opcodes" () OP_ f-op ('
		OPCODES.each do |op|
			file.puts "(\"#{op.name.to_s.upcase}\" #{i})"
			i += 1
		end
		file.puts '))'
	end
elsif(mode == 'ccore')
	open(filename, 'w') do |file|
		writeCCoreInstructions(file) do
			file.puts 'enum {'
			REGISTERS.each do |r|
				file.puts "\tREG_#{r},"
			end
			file.puts '};'
		end
	end
elsif(mode == 'jcore')
	open(filename, 'w') do |file|
		writeCCoreInstructions(file) do
			REGISTERS.each_with_index do |r, i|
				file.puts "#define REG_#{r} #{i}"
			end
		end
	end
elsif(mode == 'cscore')
	open(filename, 'w') do |file|
		i = 0
		file.puts 'namespace MoSync {'
		file.puts "public class CoreGen {"
		file.puts
		file.puts "public class Op {"
		OPCODES.each do |op|
			file.puts "\tpublic const byte #{op.name.to_s.upcase} = #{sprintf('0x%x', i)};"
			i += 1
		end
		file.puts '}'
		file.puts
		file.puts 'public class Reg {'
		REGISTERS.each_with_index do |r, i|
			file.puts "\tpublic const int #{r} = #{i};"
		end
		file.puts '}'
		file.puts
		file.puts '}'
		file.puts '}'
	end
elsif(mode == 'binutils/desc')
	open(filename, 'w') do |file|
		file.puts '#ifndef _MAPIP2_DESC_H'
		file.puts "#error Don't include this file unless you know what you're doing."
		file.puts '#endif'
		file.puts
		# plain opcode table
		file.puts 'const mapip2_insn mapip2_insns[] = {'
		OPCODES.each do |op|
			file.puts "{ OP_#{op.name.to_s.upcase}, \"#{op.mnemonic}\", { "+
				op.operands.collect { |r| r.type.to_s.upcase + ', ' }.join + "END } },"
		end
		file.puts '};'
		file.puts
		# parse tree
		# eg: 'ld' => [
		#	rd => [rs, imm, adaddr],
		#	adaddr => [rs],
		# ]
		tree = {}
		OPCODES.each do |op|
			node = tree[op.mnemonic]
			node = {} if(!node)
			n0 = node
			n2 = node
			nr = nil
			op.operands.each do |r|
				n1 = n0[r]
				n1 = {} if(!n1)
				n0[r] = n1
				n2 = n0
				nr = r
				n0 = n1
			end
			if(nr)
				n2[nr] = op.name
			else
				node = op.name
			end
			tree[op.mnemonic] = node
		end
		p tree
		puts "Mnemonics: #{tree.size}"
		tree.each do |m, op|
			outputParseNode(file, m.to_s, op)
		end
		# todo: use gperf to make a perfect hash table instead of an array.
		file.puts 'const mapip2_mnemonic mapip2_mnemonics[] = {'
		tree.each do |m, op|
			file.puts "{ \"#{m}\", #{nodePart("_#{m}", op)} },"
		end
		file.puts '};'
		writeRegisterNames(file)
	end
elsif(mode == 'regnames')
	open(filename, 'w') do |file|
		writeRegisterNames(file)
	end
else
	raise "Invalid mode: #{mode}"
end
